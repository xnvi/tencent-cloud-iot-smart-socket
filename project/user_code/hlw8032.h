#ifndef __HLW8032_H
#define __HLW8032_H

#include <stdint.h>

typedef struct {
	float current;
	float voltage;
	float power_factor;
	float active_power; // 有功功率
	float apparent_power; // 视在功率
	uint32_t total_wh; // 总耗电量，注意这里的单位是wh
}electric_info;

void hlw8032_init(uint32_t total_wh); // 初始化时传入累计用电量，之后会在这个基础上累加
void hlw8032_recv(uint8_t dat);
void hlw8032_get_info(electric_info *p_info);

#endif
