#ifndef _ADC_CALI_H_
#define _ADC_CALI_H_

#include "esp_err.h"
#include <stdint.h>

struct adc_cali_scheme_t
{

    /**
     * @brief Convert ADC raw data to calibrated voltage
     *
     * @param[in]  arg        ///< ADC calibration scheme specific context
     * @param[in]  raw        ///< ADC raw data
     * @param[out] voltage    ///< Calibrated ADC voltage (in mV)
     *
     * @return
     *         - ESP_OK:                On success
     *         - ESP_ERR_INVALID_ARG:   Invalid argument
     *         - ESP_ERR_INVALID_STATE: Invalid state, scheme didn't registered
     */
    esp_err_t (*raw_to_voltage)(void *arg, int raw, int *voltage);

    /**
     * @brief ADC calibration specific contexts
     * Can be customized to difference calibration schemes
     */
    void *ctx;
};
typedef struct adc_cali_scheme_t *adc_cali_handle_t;

typedef struct adc_oneshot_unit_ctx_t
{
    uint32_t unit_id;
} adc_oneshot_unit_ctx_t;

typedef enum adc_one_shot_clk_src
{
    ADC_CLK_SRC_APB = 0, ///< ADC clock source is APB clock
    ADC_CLK_SRC_PLL = 1, ///< ADC clock source is PLL
} adc_oneshot_clk_src_t;

typedef struct
{
    adc_unit_t unit_id;            ///< ADC unit
    adc_oneshot_clk_src_t clk_src; ///< Clock source
    adc_ulp_mode_t ulp_mode;       ///< ADC controlled by ULP, see `adc_ulp_mode_t`
} adc_oneshot_unit_init_cfg_t;

typedef struct adc_oneshot_unit_ctx_t *adc_oneshot_unit_handle_t;

typedef enum
{
    ADC_UNIT_1, ///< SAR ADC 1
    ADC_UNIT_2, ///< SAR ADC 2
} adc_unit_t;

/**
 * @brief ADC channels
 */
typedef enum
{
    ADC_CHANNEL_0, ///< ADC channel
    ADC_CHANNEL_1, ///< ADC channel
    ADC_CHANNEL_2, ///< ADC channel
    ADC_CHANNEL_3, ///< ADC channel
    ADC_CHANNEL_4, ///< ADC channel
    ADC_CHANNEL_5, ///< ADC channel
    ADC_CHANNEL_6, ///< ADC channel
    ADC_CHANNEL_7, ///< ADC channel
    ADC_CHANNEL_8, ///< ADC channel
    ADC_CHANNEL_9, ///< ADC channel
} adc_channel_t;

/**
 * @brief ADC attenuation parameter. Different parameters determine the range of the ADC.
 */
typedef enum
{
    ADC_ATTEN_DB_0 = 0,   ///< No input attenuation, ADC can measure up to approx.
    ADC_ATTEN_DB_2_5 = 1, ///< The input voltage of ADC will be attenuated extending the range of measurement by about 2.5 dB (1.33 x)
    ADC_ATTEN_DB_6 = 2,   ///< The input voltage of ADC will be attenuated extending the range of measurement by about 6 dB (2 x)
    ADC_ATTEN_DB_11 = 3,  ///< The input voltage of ADC will be attenuated extending the range of measurement by about 11 dB (3.55 x)
} adc_atten_t;

typedef enum
{
    ADC_BITWIDTH_DEFAULT = 0, ///< Default ADC output bits, max supported width will be selected
    ADC_BITWIDTH_9 = 9,       ///< ADC output width is 9Bit
    ADC_BITWIDTH_10 = 10,     ///< ADC output width is 10Bit
    ADC_BITWIDTH_11 = 11,     ///< ADC output width is 11Bit
    ADC_BITWIDTH_12 = 12,     ///< ADC output width is 12Bit
    ADC_BITWIDTH_13 = 13,     ///< ADC output width is 13Bit
} adc_bitwidth_t;

typedef enum
{
    ADC_ULP_MODE_DISABLE = 0, ///< ADC ULP mode is disabled
    ADC_ULP_MODE_FSM = 1,     ///< ADC is controlled by ULP FSM
    ADC_ULP_MODE_RISCV = 2,   ///< ADC is controlled by ULP RISCV
} adc_ulp_mode_t;

typedef struct
{
    adc_unit_t unit_id;      ///< ADC unit
    adc_channel_t chan;      ///< ADC channel, for chips with SOC_ADC_CALIB_CHAN_COMPENS_SUPPORTED, calibration can be per channel
    adc_atten_t atten;       ///< ADC attenuation
    adc_bitwidth_t bitwidth; ///< ADC raw output bitwidth
} adc_cali_curve_fitting_config_t;

typedef struct
{
    adc_atten_t atten;       ///< ADC attenuation
    adc_bitwidth_t bitwidth; ///< ADC conversion result bits
} adc_oneshot_chan_cfg_t;

esp_err_t adc_cali_create_scheme_curve_fitting(const adc_cali_curve_fitting_config_t *config, adc_cali_handle_t *ret_handle);
esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t handle, adc_channel_t chan, int *out_raw);
esp_err_t adc_cali_raw_to_voltage(adc_cali_handle_t handle, int raw, int *voltage);
esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t *init_config, adc_oneshot_unit_handle_t *ret_unit);
esp_err_t adc_oneshot_config_channel(adc_oneshot_unit_handle_t handle, adc_channel_t channel, const adc_oneshot_chan_cfg_t *config);

#endif // _ADC_CALI_H_