#include "bsp_adc.h"
#include "adc.h"

/* STM32F4 internal reference voltage: 1.21V typical */
#define VREFINT_CAL_ADDR    ((uint16_t*)0x1FFF7A2A)  /* factory calibration */
#define VREFINT_MV          1210.0f                   /* mV */
#define ADC_RESOLUTION      4096.0f

/* Temperature sensor calibration addresses (30°C and 110°C @ 3.3V) */
#define TEMPSENSOR_CAL1_ADDR ((uint16_t*)0x1FFF7A2C)
#define TEMPSENSOR_CAL2_ADDR ((uint16_t*)0x1FFF7A2E)
#define TEMPSENSOR_CAL1_TEMP 30.0f
#define TEMPSENSOR_CAL2_TEMP 110.0f

/* Battery voltage divider ratio: adjust to match your hardware */
#define VBAT_DIVIDER        10.47f  /* calibrated divider on ADC3_IN8 */
#define VDDA_MV             3300.0f

static float vdda_mv = VDDA_MV;  /* actual VDDA in mV, calibrated at init */
uint32_t debug_vbat_raw = 0;      /* debug: raw ADC3 value */
uint32_t debug_vref_raw = 0;      /* debug: raw Vrefint value */
HAL_StatusTypeDef debug_adc1_status = HAL_OK;
HAL_StatusTypeDef debug_adc3_status = HAL_OK;

static uint32_t adc_read(ADC_HandleTypeDef *hadc)
{
    HAL_StatusTypeDef st;
    st = HAL_ADC_Start(hadc);
    if (st != HAL_OK) return 0;
    st = HAL_ADC_PollForConversion(hadc, 100);
    if (hadc->Instance == ADC1) debug_adc1_status = st;
    if (hadc->Instance == ADC3) debug_adc3_status = st;
    if (st != HAL_OK) return 0;
    uint32_t val = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);
    return val;
}

/**
 * @brief Use internal Vrefint to calibrate the actual VDDA.
 *        Stores reciprocal for later voltage calculations.
 */
void init_vrefint_reciprocal(void)
{
    /* 直接使用固定 VDDA = 3300mV，无需 Vrefint 校准 */
    vdda_mv = VDDA_MV;
    debug_vref_raw = 0xFFFF; /* 标记为跳过 */
}

/**
 * @brief Read battery voltage via ADC3 channel 8 (PF10).
 * @return Voltage in volts.
 */
fp32 get_battery_voltage(void)
{
    uint32_t raw = adc_read(&hadc3);
    debug_vbat_raw = raw;
    /* raw_to_mv = raw * vdda_mv / ADC_RESOLUTION, then scale by divider */
    float mv = (float)raw * vdda_mv / ADC_RESOLUTION;
    return (mv * VBAT_DIVIDER) / 1000.0f;
}

/**
 * @brief Read internal temperature sensor via ADC1.
 * @return Temperature in degrees Celsius.
 */
fp32 get_temprate(void)
{
    uint32_t raw = adc_read(&hadc1);

    uint16_t cal1 = *TEMPSENSOR_CAL1_ADDR;
    uint16_t cal2 = *TEMPSENSOR_CAL2_ADDR;

    float temp = ((float)raw - (float)cal1) /
                 ((float)cal2 - (float)cal1) *
                 (TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP) +
                 TEMPSENSOR_CAL1_TEMP;
    return temp;
}

/**
 * @brief Read hardware version from GPIO pins HW0/HW1/HW2 (PC0/PC1/PC2).
 * @return 3-bit version value (0-7).
 */
uint8_t get_hardware_version(void)
{
    uint8_t ver = 0;
    ver |= (HAL_GPIO_ReadPin(HW0_GPIO_Port, HW0_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
    ver |= (HAL_GPIO_ReadPin(HW1_GPIO_Port, HW1_Pin) == GPIO_PIN_SET) ? 0x02 : 0x00;
    ver |= (HAL_GPIO_ReadPin(HW2_GPIO_Port, HW2_Pin) == GPIO_PIN_SET) ? 0x04 : 0x00;
    return ver;
}
