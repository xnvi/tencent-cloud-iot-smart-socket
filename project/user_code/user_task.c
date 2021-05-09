#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_hal.h"

#include "tos_k.h"
#include "user_task.h"
#include "user_utils.h"
#include "hlw8032.h"
#include "key.h"
#include "ws2812_driver.h"

uint32_t g_do_upload_property = 0;
electric_info g_power_info = {0.0, 0.0, 0.0, 0.0, 0.0, 0};
uint32_t g_switch = 0;
uint32_t g_count_down = 0;
uint32_t g_count_down_update = 0;
uint32_t g_overcurrent_event = 0;
static uint32_t g_overcurrent_lock = 0;


void user_task(void *arg)
{
	uint32_t key = 0;
	uint32_t key_timer_old = 0;
	uint32_t key_timer_new = 0;
	uint32_t wh_timer_old = 0;
	uint32_t time_sec_old = 0; // 秒级更新时间
	uint32_t power_timer_old = 0;
	uint32_t total_wh = 0;
	uint32_t color = 0;
	int32_t i = 0;

	printf("user task start\r\n");

	relay_switch(0);
  	ws2812_reset();
  	esp8266_rst();
	
	total_wh = read_wh();
	hlw8032_init(total_wh);
	
	time_sec_old = HAL_GetTick() / 1000;
	wh_timer_old = HAL_GetTick();
	power_timer_old = HAL_GetTick();
	g_do_upload_property = 1;

	while (1)
	{
		// 每隔一定时间读取新的电力信息
		if (HAL_GetTick() - power_timer_old > 2000)
		{
			power_timer_old = HAL_GetTick();
			hlw8032_get_info(&g_power_info);
			// if (g_power_info.active_power < 1200.0)
			// {
			// 	g_power_info.active_power += 0.1;
			// }
		}

		// 保存累计电力，延长flash寿命，只有每分钟且用电量变化时才写入
		// if (HAL_GetTick() - wh_timer_old > 60 * 1000 && total_wh != g_power_info.total_wh)
		if (0)
		{
			wh_timer_old = HAL_GetTick();
			total_wh = g_power_info.total_wh;
			save_wh(total_wh);
		}

		key = key_read();
		// key = key_hw_read(0);
		if (key)
		{
			// printf("key 0x%x \r\n", key);
			key_timer_old = HAL_GetTick();
			while (1)
			{
				tos_task_delay(tos_millisec2tick(31));
				key = key_read();
				if (key == 0)
				{
					break;
				}
			}
			key_timer_new = HAL_GetTick();
			if (key_timer_new - key_timer_old < 1000)
			{
				// 开关
				printf("switch %d \r\n", g_switch);
				g_switch = !g_switch;
			}
			else if (key_timer_new - key_timer_old > 5000 && key_timer_new - key_timer_old < 10000)
			{
				// 配网
				printf("wifi config\r\n");
				wifi_config();
			}
			else if (key_timer_new - key_timer_old > 10000 && key_timer_new - key_timer_old < 15000)
			{
				// 恢复出厂设置
				printf("config reset\r\n");
				// erase_wh();
				ws2812_write_color_u32(0x00000000);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00FFFFFF);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00000000);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00FFFFFF);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00000000);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00FFFFFF);
				HAL_Delay(500);
				ws2812_write_color_u32(0x00000000);
			}
			else
			{
				// 无效按键
			}
			key_timer_old = 0;
			key_timer_new = 0;
		}
		// printf("key 0x%x \r\n", key);

		// 更新倒计时
		if (g_count_down > 0 && time_sec_old != HAL_GetTick() / 1000)
		{
			g_count_down -= 1;
			time_sec_old = HAL_GetTick() / 1000;
		}
		if (g_count_down == 0 && g_count_down_update)
		{
			g_count_down_update = 0;
			g_switch = !g_switch;
		}

		// 电流超过5A强制断电，设置4.9为了更保险一点
		if (g_power_info.current > 4.9)
		// if(HAL_GetTick() > 60000 && HAL_GetTick() < 63000)
		{
			g_switch = 0;
			// 使用 DeviceStatus 事件发送告警信息
			// 信息内容为 "overcurrent"
			g_overcurrent_event = 1;
			g_overcurrent_lock = 1;
		}

		// 过流以后一直闪红灯，直到再次打开开关
		if (g_overcurrent_lock)
		{
			ws2812_write_color_u32(0x00FF0000);
			HAL_Delay(500);
			ws2812_write_color_u32(0x00000000);
			HAL_Delay(500);
			if (g_switch)
			{
				g_overcurrent_lock = 0;
			}
		}
		else
		{
			// 通过呼吸灯的颜色来表示功率
			if(g_switch)
			{
				// 色环0-1536，颜色依次是红黄绿青蓝紫红
				// 0红，255黄，512绿，768青，1024蓝，1280紫，1536红
				if (g_power_info.active_power > 850.0)
				{
					color_breathe(0x00FF0000);
				}
				else
				{
					color = color_ring(850 - (int)g_power_info.active_power);
					color_breathe(color);
				}
			}
			else
			{
				ws2812_write_color_u32(0x0000FF00);
			}
		}

		// 设置继电器状态
		relay_switch(g_switch);

		tos_task_delay(tos_millisec2tick(7));
	}
}
