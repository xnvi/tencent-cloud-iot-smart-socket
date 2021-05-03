#ifndef __HLW8032_H
#define __HLW8032_H

#include <stdint.h>

// void hlw8032_init(void);
void hlw8032_recv(uint8_t dat);
void hlw8032_analyse(void);
// float hlw8032_get_current(void);
// float hlw8032_get_voltage(void);
// float hlw8032_get_power_factor(void);
// float hlw8032_get_active_power(void);
// float hlw8032_get_apparent_power(void);
void hlw8032_get_info(void);

float hlw8032_get_total_kwh(void);
void hlw8032_save_total_kwh(void);

#endif
