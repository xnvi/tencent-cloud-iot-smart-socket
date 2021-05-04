#include "hlw8032.h"

#include "stm32g0xx_ll_gpio.h"
#include "user_utils.h"

#include <stdio.h>
#include <string.h>

/*
测试数据
状态寄存器
检测寄存器
电压参数寄存器
电压寄存器
电流参数寄存器
电流寄存器
功率参数寄存器
功率寄存器
数据更新寄存器
PF寄存器
校验和寄存器

0xF2
0x5A
0x02DA78
0x072E2C
0x003DD6
0x072D94
0x4E0C78
0x557BEE
0x61
0x0003
0x84
*/

// 电压、电流、功率系数，如果更换采样电阻、互感器等采样相关电路需要重新计算并校准，详见官方手册
// static const float voltage_coefficient = 1.0;
// static const float current_coefficient = 1.0;
// static const float power_coefficient = 1.0;

static const float voltage_coefficient = 3.069;
static const float current_coefficient = 1.007;
static const float power_coefficient = 3.108;

typedef struct {
	uint8_t state;
	uint8_t check;
	uint8_t data_updata;
	uint8_t checksum;
	uint32_t voltage_param;
	uint32_t voltage;
	uint32_t current_param;
	uint32_t current;
	uint32_t power_param;
	uint32_t power;
	uint16_t pf;
}hlw8032_reg;

static hlw8032_reg reg;
static electric_info info;

static uint8_t recv_buf[24];
// static uint8_t recv_buf[24] = {0xF2,0x5A,0x02,0xDA,0x78,0x07,0x2E,0x2C,0x00,0x3D,0xD6,0x07,0x2D,0x94,0x4E,0x0C,0x78,0x55,0x7B,0xEE,0x61,0x00,0x03,0x84};

static uint8_t last_data_updata;
static uint16_t last_pf;
static int32_t kwh_pf_cnt = 0; // 每kwh的pf计数
static int32_t wh_pf_cnt = 0; // 每wh的pf计数
static int32_t pf_cnt = 0;

static void hlw8032_analyse(void)
{
	uint8_t check_sum = 0;
	uint8_t i = 0;

	for (i = 2; i < 23; i++)
	{
		check_sum += recv_buf[i];
	}

	if (check_sum != recv_buf[23])
	{
		return;
	}

	reg.state         = recv_buf[0];
	reg.check         = recv_buf[1];
	reg.voltage_param = (recv_buf[2] << 16) + (recv_buf[3] << 8) + recv_buf[4];
	reg.voltage       = (recv_buf[5] << 16) + (recv_buf[6] << 8) + recv_buf[7];
	reg.current_param = (recv_buf[8] << 16) + (recv_buf[9] << 8) + recv_buf[10];
	reg.current       = (recv_buf[11] << 16) + (recv_buf[12] << 8) + recv_buf[13];
	reg.power_param   = (recv_buf[14] << 16) + (recv_buf[15] << 8) + recv_buf[16];
	reg.power         = (recv_buf[17] << 16) + (recv_buf[18] << 8) + recv_buf[19];
	reg.data_updata   = recv_buf[20];
	reg.pf            = (recv_buf[21] << 8) + recv_buf[22];
	reg.checksum      = recv_buf[23];

	if (kwh_pf_cnt == 0 || wh_pf_cnt == 0)
	{
		// 1 度电的脉冲数量 = (1 / 功率参数寄器) x (1 / (电压系数 x 电流系数)) x 10^9 x 3600
		kwh_pf_cnt = (1000000.0 / reg.power_param) * (100000.0 / voltage_coefficient / current_coefficient) * 36.0; // 每1kwh的pf计数
		wh_pf_cnt = kwh_pf_cnt / 1000; // 每1wh的pf计数
	}

	// 1. 当 State REG = 0xaa 时，芯片误差修正功能失效，此时电压参数寄存器、电流参数寄存器和功率参数寄存器不可用;
	// 2. 当 State REG = 0x55 时，芯片误差修正功能正常，此时电压参数寄存器、电流参数寄存器和功率参数寄存器可用,且
	// 电压寄存器、电流寄存器和功率寄存器未溢出;
	// 3. 当 State REG = 0xfx 时，芯片误差修正功能正常, 此时电压参数寄存器、电流参数寄存器和功率参数寄存器可用, 
	// State REG 的相应位为 1 时表示相应的寄存器溢出，溢出表示电流、电压或功率值非常小，接近 0;
	if (reg.state == 0xAA || (reg.state & 0xF1) == 0xF1)
	{
		// info.state = 1;
		info.voltage = 0.0;
		info.current = 0.0;
		info.active_power = 0.0;
		info.apparent_power = 0.0;
		info.power_factor = 0.0;
		return;
	}
	
	// info.state = 0;
	if (reg.state == 0x55)
	{
		info.voltage = voltage_coefficient * reg.voltage_param / reg.voltage;
		info.current = current_coefficient * reg.current_param / reg.current;
		info.active_power = power_coefficient * reg.power_param / reg.power;
		info.apparent_power = info.voltage * info.current;
		info.power_factor = info.active_power / info.apparent_power;

		if (info.power_factor > 1.0)
		{
			info.power_factor = 1.0;
		}
	}

	if ((reg.state & 0xF8) == 0xF8)
	{
		info.voltage = voltage_coefficient * reg.voltage_param / reg.voltage;
	}
	if ((reg.state & 0xF4) == 0xF4)
	{
		info.current = current_coefficient * reg.current_param / reg.current;
	}
	
	// 功率寄存器不可用时，只更本地pf新计数器，不统累计用电量
	if ((reg.state & 0xF2) == 0xF2)
	{
		info.active_power = 0.0;
		info.apparent_power = 0.0;
		info.power_factor = 0.0;

		last_pf = reg.pf;
		return;
	}

	// data_updata 的 bit7每变化一次，说明pf溢出
	if (((last_data_updata ^ reg.data_updata) & 0x80) == 0x80)
	{
		pf_cnt += 65536 - last_pf + reg.pf;
	}
	else
	{
		pf_cnt += reg.pf - last_pf;
	}
	last_pf = reg.pf;

	// 计算累计用电量
	if (pf_cnt > wh_pf_cnt)
	{
		pf_cnt -= wh_pf_cnt;
		info.total_wh += 1;
	}
}

void hlw8032_init(uint32_t total_wh)
{
	// GPIO init
	// 1 度电的脉冲数量 = (1 / 功率参数寄器) x (1 / (电压系数 x 电流系数)) x 10^9 x 3600
	// skwh_pf_cnt = (10000000 / reg.power_param) * (100000 / voltage_coefficient / current_coefficient) * 36;
	// kwh_pf_cnt = 123456; // 每1kwh的pf计数
	// wh_pf_cnt = kwh_pf_cnt / 1000; // 每1kwh的pf计数

	info.total_wh = total_wh;
}

// 在串口接收中断函数中调用
void hlw8032_recv(uint8_t dat)
{
	static uint32_t last_tick = 0;
	static uint32_t pos = 0;
	uint32_t new_tick = 0;

	new_tick = HAL_GetTick();

	// hlw8032 每50ms发送一次数据，每包数据24字节
	if (new_tick - last_tick > 20)
	{
		pos = 0;
	}
	last_tick = new_tick;
	recv_buf[pos] = dat;
	pos += 1;
	if (pos == 24)
	{
		hlw8032_analyse();
		pos = 0;
	}
}

void hlw8032_get_info(electric_info *p_info)
{
	// printf("sta 0x%x, chk 0x%x, vp 0x%x, v 0x%x, cp 0x%x, c 0x%x, pp 0x%x, p 0x%x, du 0x%x, pf 0x%x \r\n",
	// 	reg.state, reg.check,
	// 	reg.voltage_param, reg.voltage,
	// 	reg.current_param, reg.current,
	// 	reg.power_param, reg.power,
	// 	reg.data_updata, reg.pf);

	// printf("c %f, v %f, pf %f, p %f, ap %f, wh %d \r\n",
	// info.current, info.voltage, info.power_factor, info.active_power, info.apparent_power, info.total_wh);

	// printf("\r\n\r\n");

	memcpy(p_info, &info, sizeof(electric_info));

	return;
}
