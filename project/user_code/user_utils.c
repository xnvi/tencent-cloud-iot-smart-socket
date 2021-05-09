#include <stdio.h>

#include "user_utils.h"
#include "stm32g0xx_hal_flash.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_tim.h"
#include "hlw8032.h"
#include "ws2812_driver.h"
#include "at_client.h"
#include "tos_k.h"

/*
����ԭ�ͣ�void delay_us(u32 n)
�������ܣ���ʱ΢��
��    ����n ��ʱ��΢����
�� �� ֵ����
��    ע�����ʹ����HAL�⣬����ñ���������΢����ʱ�����û��ʹ��HAL�⣬���������ܱ�֤��ʱ����ȷ�ԣ�
          �����еĲ��ֲ��������MCU��Ƶ�����޸�
*/
void delay_us(uint32_t n)
{
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;//LOAD��ֵ
	ticks = n * 64;//��Ҫ�Ľ�������72Mhz
	told = SysTick->VAL;//�ս���ʱ�ļ�����ֵ

	// TODO ��ϵͳ��������ע�����
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told)
			{
				tcnt += told - tnow;//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			
			if(tcnt >= ticks)
			{
				break;//ʱ�䳬��������Ҫ�ӳٵ�ʱ��,���˳�.
			}
		}
	}
}

uint32_t color_ring(int32_t index)
{
	uint8_t r = 0, g = 0, b = 0;
	uint8_t hi, lo;

	hi = index >> 8;
	lo = index & 0x000000FF;

	switch (hi)
	{
		case 0:
			r = 255;
			g = lo;
			b = 0;
			break;
		case 1:
			r = 255 - lo;
			g = 255;
			b = 0;
			break;
		case 2:
			r = 0;
			g = 255;
			b = lo;
			break;
		case 3:
			r = 0;
			g = 255 - lo;
			b = 255;
			break;
		case 4:
			r = lo;
			g = 0;
			b = 255;
			break;
		case 5:
			r = 255;
			g = 0;
			b = 255 - lo;
			break;
		
		default:
			break;
	}

	return (r << 16) + (g << 8) + b;
}


// ��ɫ������������HSLɫ���RGBɫ��
#define _MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define _MIN(a,b)  (((a) < (b)) ? (a) : (b))

static void RGB2HSL(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *l)
{
	float fr, fg, fb, max, min, delta, sum;
	fr = r / 255.0;
	fg = g / 255.0;
	fb = b / 255.0;
	max = _MAX(fr, _MAX(fg, fb));
	min = _MIN(fr, _MIN(fg, fb));
	delta = max - min;
	sum = max + min;

	//�������������ƽ�������������޷�����
	//ave = (a&b)+((a^b)>>1)
	//hsl[2] = ((max & min) + ((max ^ min) >> 1)) / 255.0;
	*l = sum / 2;
	if(max == min)
	{
		*h = *s = 0;
	}
	else
	{
		*s = *l > 0.5 ? delta / (2 - sum) : delta / sum;

		//�Ƕ���0-1��ʾ
		/*
		if(max == fr)
		{
			*h = (fg - fb) / delta + (fg < fb ? 6 : 0);
		}
		else if(max == fg)
		{
			*h = (fb - fr) / delta + 2;
		}
		else if(max == fb)
		{
			*h = (fr - fg) / delta + 4;
		}
		*h /= 6;
		// *h *= 360;
		*/

		//�Ƕ���0-360��ʾ
		if(max == fr)
		{
			*h = (fg - fb) / delta * 60 + (fg > fb ? 0 : 360);
		}
		else if(max == fg)
		{
			*h = (fb - fr) / delta * 60 + 120;
		}
		else if(max == fb)
		{
			*h = (fr - fg) / delta * 60 + 240;
		}
	}

	return;
}

static uint8_t pqt2rgb(float p, float q, float t)
{
	if(t < 0) t += 1;
	if(t > 1) t -= 1;
	if(t < 1 / 6.0) return (uint8_t)((p + (q - p) * 6 * t) * 255);
	if(t < 1 / 2.0) return (uint8_t)(q * 255);
	if(t < 2 / 3.0) return (uint8_t)((p + (q - p) * (2 / 3.0 - t) * 6) * 255);
	return (uint8_t)(p * 255);
}

//hΪ0-360
static void HSL2RGB(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b)
{
	float q, p, hk;

	if(s == 0)
	{
		*r = *g = *b = (uint8_t)(l * 255);
	}
	else
	{
		q = (l <= 0.5) ? (l * (1 + s)) : (l + s - (l * s));
		p = l + l - q;
		hk = h / 360.0;

		*r = pqt2rgb(p, q, hk + 1.0 / 3.0);
		*g = pqt2rgb(p, q, hk);
		*b = pqt2rgb(p, q, hk - 1.0 / 3.0);
	}

	return;
}

void color_breathe(uint32_t color)
{
	static uint32_t aim_color = 0;
	static float aim_l = 0.0;
	float h = 0.0, s = 0.0, l = 0.0;
	uint32_t now_color = 0;
	static uint8_t dir = 0; // 1������0�䰵

	aim_color = color;
	RGB2HSL((aim_color >> 16) & 0xFF, (aim_color >> 8) & 0xFF, (aim_color & 0xFF), &h, &s, &l);

	if (dir)
	{
		aim_l += 0.003;
	}
	else
	{
		aim_l -= 0.003;
	}

	if (aim_l > 0.5)
	{
		aim_l = 0.5;
		dir = 0;
	}

	if (aim_l < 0.0)
	{
		aim_l = 0.0;
		dir = 1;
	}
	HSL2RGB(h, s, aim_l, &((uint8_t *)(&now_color))[2], &((uint8_t *)(&now_color))[1], &((uint8_t *)(&now_color))[0]);
	ws2812_write_color_u32(now_color);
}


void relay_switch(int32_t sw)
{
	if(sw)
	{
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);
	}
	else
	{
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
	}
}

void esp8266_rst(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
	HAL_Delay(10);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);
}

// ����flash���������һ��8�ֽڶ���ġ���Ϊ0�����ݵĵ�ַ
// ���ȫΪFF�򷵻�NULL
// ���ȫ��ΪFF�򷵻�ĩ��ַ
uint32_t *flash_search(void)
{
	uint64_t *ptr = (uint64_t *)USER_FLASH_ADDR;
	int32_t low, high, mid, max;

	low = 0;
	high = USER_FLASH_SIZE / sizeof(uint64_t);
	max = high;

	while (low <= high)
	{
		mid = low + (high - low) / 2;

		if (mid == max)
		{
			return (uint32_t *)&ptr[mid];
		}
		else if (ptr[mid] != 0xFFFFFFFFFFFFFFFF && ptr[mid+1] == 0xFFFFFFFFFFFFFFFF)
		{
			return (uint32_t *)&ptr[mid];
		}
		else if (ptr[mid] == 0xFFFFFFFFFFFFFFFF)
		{
			high = mid - 1;
		}
		else if (ptr[mid] != 0xFFFFFFFFFFFFFFFF)
		{
			low = mid + 1;
		}
		else
		{
			// error
			// return NULL;
		}

		// if (aim == now)
		// {
		// 	return (uint8_t*)&font12_table[mid].fontdata;
		// }
		// else if (aim < now)
		// {
		// 	high = mid - 1;
		// }
		// else
		// {
		// 	low = mid + 1;
		// }	
	}

	return NULL;
}

void erase_wh()
{
	FLASH_EraseInitTypeDef pEraseInit = {FLASH_TYPEERASE_PAGES, USER_FLASH_OFFSET / FLASH_PAGE_SIZE, USER_FLASH_SIZE / FLASH_PAGE_SIZE};
	uint32_t PageError = 0;

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pEraseInit, &PageError);
	HAL_FLASH_Lock();
	return;
}

void save_wh(uint32_t wh)
{
	uint32_t *ptr = NULL;

	ptr = flash_search();

	if (ptr == NULL)
	{
		ptr = (uint32_t *)USER_FLASH_ADDR;
		// return;
	}
	else if (ptr == (uint32_t *)(USER_FLASH_ADDR + USER_FLASH_SIZE - sizeof(uint64_t)))
	{
		erase_wh();
		ptr = (uint32_t *)USER_FLASH_ADDR;
	}
	else
	{
		ptr += 1;
	}

	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)ptr, (uint64_t)wh);
	HAL_FLASH_Lock();

	return;
}

int32_t read_wh(void)
{
	uint32_t *ptr = NULL;

	ptr = flash_search();

	if (ptr == NULL)
	{
		return 0;
	}

	return (int32_t)*ptr;
}


extern uint32_t g_do_upload_property;
void wifi_config(void)
{
	int i;
	at_response_t resp = NULL;

	g_do_upload_property = 0;

	ws2812_write_color_u32(0x00000000);
	tos_sleep_ms(500);
	ws2812_write_color_u32(0x000000FF);
	tos_sleep_ms(500);
	ws2812_write_color_u32(0x00000000);
	tos_sleep_ms(500);
	ws2812_write_color_u32(0x000000FF);
	tos_sleep_ms(500);

	resp = at_create_resp(64, 0, 2000);

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCMQTTDISCONN"))
	{
		// ��ʱ���ʧ�ܣ���ʵ��ִ�гɹ���
		printf("cmd AT+TCMQTTDISCONN exec err");
	}

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+CWQAP"))
	{
		// ��ʱ���ʧ�ܣ���ʵ��ִ�гɹ���
		printf("cmd AT+CWQAP exec err");
	}
	
	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTARTSMART"))
	{
		// ��ʱ���ʧ�ܣ���ʵ��ִ�гɹ���
		printf("cmd AT+TCSTARTSMART exec err");
	}

	// �ȴ������յ����������˵��������ɣ�����Ϊ�˼򵥲�������
	// Smartconfig connected Wi-Fi

	tos_sleep_ms(1000);

	// �ȴ�60�룬��������
	for(i=0; i<60; i++)
	{
		if (i % 2)
		{
			ws2812_write_color_u32(0x000000FF);
		}
		else
		{
			ws2812_write_color_u32(0x00000000);
		}

		tos_sleep_ms(500);
	}

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTOPSMART"))
	{
		// ��ʱ���ʧ�ܣ���ʵ��ִ�гɹ���
		printf("cmd AT+TCSTOPSMART exec err");
	}

	if(resp)
	{
		at_delete_resp(resp);
	}

	// �������۳ɹ���񶼻�ϵͳ��λ
	HAL_NVIC_SystemReset();
}
