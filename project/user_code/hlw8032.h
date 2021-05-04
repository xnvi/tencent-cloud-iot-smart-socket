#ifndef __HLW8032_H
#define __HLW8032_H

#include <stdint.h>

typedef struct {
	float current;
	float voltage;
	float power_factor;
	float active_power; // �й�����
	float apparent_power; // ���ڹ���
	uint32_t total_wh; // �ܺĵ�����ע������ĵ�λ��wh
}electric_info;

void hlw8032_init(uint32_t total_wh); // ��ʼ��ʱ�����ۼ��õ�����֮���������������ۼ�
void hlw8032_recv(uint8_t dat);
void hlw8032_get_info(electric_info *p_info);

#endif
