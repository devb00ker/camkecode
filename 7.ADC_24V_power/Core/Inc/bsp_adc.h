#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef float fp32;

void    init_vrefint_reciprocal(void);
fp32    get_battery_voltage(void);
fp32    get_temprate(void);
uint8_t get_hardware_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H__ */
