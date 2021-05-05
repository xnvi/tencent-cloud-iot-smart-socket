#include "user_utils.h"
#include "stm32g0xx_hal_flash.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_tim.h"
#include "hlw8032.h"
#include "user_config.h"

// #include "encoder.h"

// #include "stm32f1xx_hal.h"

// DEVICE_INFO my_device_info;

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

void get_device_info()
{
	// TODO ������������flash��ȡ�豸��Ϣ
	// uint64_t *ptr = (uint64_t *)DEVICE_INFO_FLASH_ADDR;

	// strcpy(my_device_info.product_id, USER_PRODUCT_ID);
	// strcpy(my_device_info.device_name, USER_DEVICE_NAME);
	// strcpy(my_device_info.device_secret, USER_DEVICE_SECRET);
}
