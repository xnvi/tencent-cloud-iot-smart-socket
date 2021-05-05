#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

// 按键数设置
#define KEY_TOTAL_NUM 1

#if KEY_TOTAL_NUM > 16
#error "too many key"
#elif KEY_TOTAL_NUM > 8
typedef uint8_t key_data;
#define KEY_STATE_MASK 0x55555555
#elif KEY_TOTAL_NUM > 4
typedef uint16_t key_data;
#define KEY_STATE_MASK 0x5555
#elif KEY_TOTAL_NUM > 0
typedef uint32_t key_data;
#define KEY_STATE_MASK 0x55
#else
#error "key num is 0"
#endif

// 状态标识
#define KEY_STATE_RELEASE 0x00
#define KEY_STATE_RISING  0x01
#define KEY_STATE_PUSH    0x03
#define KEY_STATE_FALLING 0x02

// 读取某个按键的数据
#define KEY_READ_NUM(data, n) ((data >> n * 2) & 0x03)

key_data key_hw_read(key_data key_num);
void key_init(void);
void key_scan(void);
key_data key_read(void);

#endif
