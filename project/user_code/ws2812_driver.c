#include "ws2812_driver.h"

#include "stm32g0xx_ll_gpio.h"
#include "user_utils.h"


// O0优化
// #define WS2812_DELAY1() \
// __NOP();__NOP();__NOP();__NOP();

// #define WS2812_DELAY2() \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();__NOP();__NOP();

// #define WS2812_DELAY3() \
// __NOP();__NOP();__NOP();__NOP(); \
// __NOP();__NOP();


// O3优化
#define WS2812_DELAY1() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();

#define WS2812_DELAY2() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();

#define WS2812_DELAY3() \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();__NOP();__NOP(); \
__NOP();__NOP();


void ws2812_init(void)
{
	// GPIO init
}

void ws2812_reset(void)
{
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_3);
	delay_us(10);
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_3);
	delay_us(60);
}

void ws2812_write_byte(uint8_t dat)
{
	uint8_t i = 8;

	while (i)
	{
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_3);
		WS2812_DELAY1();
		if(!(dat & 0x80))
		{
			LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_3);
		}
		WS2812_DELAY2();
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_3);
		WS2812_DELAY3();

		dat <<= 1;
		i--;
	}
}

//数组按RGB顺序存储，输出GRB
void ws2812_write_color_rgb(uint8_t *color)
{
	__disable_irq();
	ws2812_write_byte(color[1]);
	ws2812_write_byte(color[0]);
	ws2812_write_byte(color[2]);
	__enable_irq();
}

//高字节忽略，剩下三字节按RGB顺序存储，输出GRB
void ws2812_write_color_u32(uint32_t color)
{
	__disable_irq();
	// 存储格式wrgb
	ws2812_write_byte((color >> 8) & 0xFF);
	ws2812_write_byte((color >> 16) & 0xFF);
	ws2812_write_byte(color & 0xFF);
	__enable_irq();

}
