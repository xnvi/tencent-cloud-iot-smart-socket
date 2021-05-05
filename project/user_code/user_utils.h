#ifndef __USER_UTILS_H
#define __USER_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void delay_us(uint32_t n);
uint32_t color_ring(int32_t index);
void relay_switch(int32_t sw);
void esp8266_rst(void);


// 这个部分片内flash用于保存累计电量信息
// stm32各系列flash地址、大小、页大小、擦写寿命等均不相同，更换芯片时需特别注意
// flash区基址0x0800 0000
// #define FLASH_PAGE_SIZE // 已经在 stm32g0xx_hal_flash.h 定义了
#define FLASH_BASE_ADDR (0x08000000) // 片内flash基址

#define DEVICE_INFO_FLASH_OFFSET (108 * 1024) // 设备信息的片内flash地址偏移
#define DEVICE_INFO_FLASH_ADDR (FLASH_BASE_ADDR + DEVICE_INFO_FLASH_OFFSET) // 设备信息的片内flash地址
#define DEVICE_INFO_FLASH_SIZE (4 * 1024) // 设备信息的片内flash大小

#define USER_FLASH_OFFSET (112 * 1024) // 用户使用的片内flash地址偏移
#define USER_FLASH_ADDR (FLASH_BASE_ADDR + USER_FLASH_OFFSET) // 用户使用的片内flash地址
#define USER_FLASH_SIZE (16 * 1024) // 用户使用的片内flash大小

void save_wh(uint32_t wh);
int32_t read_wh(void);
void erase_wh(void);


#ifdef __cplusplus
}
#endif
#endif
