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


// �������Ƭ��flash���ڱ����ۼƵ�����Ϣ
// stm32��ϵ��flash��ַ����С��ҳ��С����д�����Ⱦ�����ͬ������оƬʱ���ر�ע��
// flash����ַ0x0800 0000
// #define FLASH_PAGE_SIZE // �Ѿ��� stm32g0xx_hal_flash.h ������
#define FLASH_BASE_ADDR (0x08000000) // Ƭ��flash��ַ

#define DEVICE_INFO_FLASH_OFFSET (108 * 1024) // �豸��Ϣ��Ƭ��flash��ַƫ��
#define DEVICE_INFO_FLASH_ADDR (FLASH_BASE_ADDR + DEVICE_INFO_FLASH_OFFSET) // �豸��Ϣ��Ƭ��flash��ַ
#define DEVICE_INFO_FLASH_SIZE (4 * 1024) // �豸��Ϣ��Ƭ��flash��С

#define USER_FLASH_OFFSET (112 * 1024) // �û�ʹ�õ�Ƭ��flash��ַƫ��
#define USER_FLASH_ADDR (FLASH_BASE_ADDR + USER_FLASH_OFFSET) // �û�ʹ�õ�Ƭ��flash��ַ
#define USER_FLASH_SIZE (16 * 1024) // �û�ʹ�õ�Ƭ��flash��С

void save_wh(uint32_t wh);
int32_t read_wh(void);
void erase_wh(void);


#ifdef __cplusplus
}
#endif
#endif
