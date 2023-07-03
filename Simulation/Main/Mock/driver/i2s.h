#ifndef _I2S_H
#define _I2S_H

#include "esp_err.h"

#include <stdint.h>
#include <stdio.h>

typedef enum
{
    I2S_NUM_0 = 0, /*!< I2S controller port 0 */
    I2S_NUM_1 = 1, /*!< I2S controller port 1 */
    I2S_NUM_AUTO,  /*!< Select whichever port is available */
} i2s_port_t;

typedef uint32_t TickType_t;

typedef enum
{
    I2S_MODE_MASTER = (0x1 << 0), /*!< Master mode*/
    I2S_MODE_SLAVE = (0x1 << 1),  /*!< Slave mode*/
    I2S_MODE_TX = (0x1 << 2),     /*!< TX mode*/
    I2S_MODE_RX = (0x1 << 3),     /*!< RX mode*/
    I2S_MODE_PDM = (0x1 << 6),    /*!< I2S PDM mode*/
} i2s_mode_t;

typedef enum
{
    I2S_BITS_PER_SAMPLE_8BIT = 8,   /*!< data bit-width: 8 */
    I2S_BITS_PER_SAMPLE_16BIT = 16, /*!< data bit-width: 16 */
    I2S_BITS_PER_SAMPLE_24BIT = 24, /*!< data bit-width: 24 */
    I2S_BITS_PER_SAMPLE_32BIT = 32, /*!< data bit-width: 32 */
} i2s_bits_per_sample_t;

typedef enum
{
    I2S_CHANNEL_FMT_RIGHT_LEFT, /*!< Separated left and right channel */
    I2S_CHANNEL_FMT_ALL_RIGHT,  /*!< Load right channel data in both two channels */
    I2S_CHANNEL_FMT_ALL_LEFT,   /*!< Load left channel data in both two channels */
    I2S_CHANNEL_FMT_ONLY_RIGHT, /*!< Only load data in right channel (mono mode) */
    I2S_CHANNEL_FMT_ONLY_LEFT,  /*!< Only load data in left channel (mono mode) */
    // Multiple channels are available with TDM feature
    I2S_CHANNEL_FMT_MULTIPLE, /*!< More than two channels are used */
} i2s_channel_fmt_t;

typedef enum
{
    I2S_COMM_FORMAT_STAND_I2S = 0X01,       /*!< I2S communication I2S Philips standard, data launch at second BCK*/
    I2S_COMM_FORMAT_STAND_MSB = 0X02,       /*!< I2S communication MSB alignment standard, data launch at first BCK*/
    I2S_COMM_FORMAT_STAND_PCM_SHORT = 0x04, /*!< PCM Short standard, also known as DSP mode. The period of synchronization signal (WS) is 1 bck cycle.*/
    I2S_COMM_FORMAT_STAND_PCM_LONG = 0x0C,  /*!< PCM Long standard. The period of synchronization signal (WS) is channel_bit*bck cycles.*/
    I2S_COMM_FORMAT_STAND_MAX,              /*!< standard max*/

    // old definition will be removed in the future.
    I2S_COMM_FORMAT_I2S = 0x01,       /*!< I2S communication format I2S, correspond to `I2S_COMM_FORMAT_STAND_I2S`*/
    I2S_COMM_FORMAT_I2S_MSB = 0x01,   /*!< I2S format MSB, (I2S_COMM_FORMAT_I2S |I2S_COMM_FORMAT_I2S_MSB) correspond to `I2S_COMM_FORMAT_STAND_I2S`*/
    I2S_COMM_FORMAT_I2S_LSB = 0x02,   /*!< I2S format LSB, (I2S_COMM_FORMAT_I2S |I2S_COMM_FORMAT_I2S_LSB) correspond to `I2S_COMM_FORMAT_STAND_MSB`*/
    I2S_COMM_FORMAT_PCM = 0x04,       /*!< I2S communication format PCM, correspond to `I2S_COMM_FORMAT_STAND_PCM_SHORT`*/
    I2S_COMM_FORMAT_PCM_SHORT = 0x04, /*!< PCM Short, (I2S_COMM_FORMAT_PCM | I2S_COMM_FORMAT_PCM_SHORT) correspond to `I2S_COMM_FORMAT_STAND_PCM_SHORT`*/
    I2S_COMM_FORMAT_PCM_LONG = 0x08,  /*!< PCM Long, (I2S_COMM_FORMAT_PCM | I2S_COMM_FORMAT_PCM_LONG) correspond to `I2S_COMM_FORMAT_STAND_PCM_LONG`*/
} i2s_comm_format_t;

typedef enum
{
    I2S_BITS_PER_CHAN_DEFAULT = (0), /*!< channel bit-width equals to data bit-width */
    I2S_BITS_PER_CHAN_8BIT = (8),    /*!< channel bit-width: 8 */
    I2S_BITS_PER_CHAN_16BIT = (16),  /*!< channel bit-width: 16 */
    I2S_BITS_PER_CHAN_24BIT = (24),  /*!< channel bit-width: 24 */
    I2S_BITS_PER_CHAN_32BIT = (32),  /*!< channel bit-width: 32 */
} i2s_bits_per_chan_t;

typedef struct
{
    int mck_io_num;   /*!< MCK pin, output */
    int bck_io_num;   /*!< BCK pin, input in slave role, output in master role */
    int ws_io_num;    /*!< WS pin, input in slave role, output in master role */
    int data_out_num; /*!< DATA pin, output */
    int data_in_num;  /*!< DATA pin, input */
} i2s_pin_config_t;

/**
 * @brief I2S driver configuration parameters
 *
 */
typedef struct
{

    i2s_mode_t mode;                        /*!< I2S work mode */
    uint32_t sample_rate;                   /*!< I2S sample rate */
    i2s_bits_per_sample_t bits_per_sample;  /*!< I2S sample bits in one channel */
    i2s_channel_fmt_t channel_format;       /*!< I2S channel format.*/
    i2s_comm_format_t communication_format; /*!< I2S communication format */
    int intr_alloc_flags;                   /*!< Flags used to allocate the interrupt. One or multiple (ORred) ESP_INTR_FLAG_* values. See esp_intr_alloc.h for more info */
    union
    {
        int dma_desc_num;                              /*!< The total number of descriptors used by I2S DMA to receive/transmit data */
        int dma_buf_count __attribute__((deprecated)); /*!< This is an alias to 'dma_desc_num' for backward compatibility */
    };
    union
    {
        int dma_frame_num;                           /*!< Frame number for one-time sampling. Frame here means the total data from all the channels in a WS cycle */
        int dma_buf_len __attribute__((deprecated)); /*!< This is an alias to 'dma_frame_num' for backward compatibility */
    };
    bool use_apll;                     /*!< I2S using APLL as main I2S clock, enable it to get accurate clock */
    bool tx_desc_auto_clear;           /*!< I2S auto clear tx descriptor if there is underflow condition (helps in avoiding noise in case of data unavailability) */
    int fixed_mclk;                    /*!< I2S using fixed MCLK output. If use_apll = true and fixed_mclk > 0, then the clock output for i2s is fixed and equal to the fixed_mclk value. If fixed_mclk set, mclk_multiple won't take effect */
    i2s_bits_per_chan_t bits_per_chan; /*!< I2S total bits in one channel， only take effect when larger than 'bits_per_sample', default '0' means equal to 'bits_per_sample' */

    uint32_t total_chan; /*!< I2S Total number of channels. If it is smaller than the biggest active channel number, it will be set to this number automatically. */
    bool left_align;     /*!< Set to enable left alignment */
    bool big_edin;       /*!< Set to enable big endian */
    bool bit_order_msb;  /*!< Set to enable msb order */
    bool skip_msk;       /*!< Set to enable skip mask. If it is enabled, only the data of the enabled channels will be sent, otherwise all data stored in DMA TX buffer will be sent */

} i2s_driver_config_t;

typedef i2s_driver_config_t i2s_config_t; // for backward compatible

esp_err_t i2s_write(i2s_port_t i2s_num, const void *src, size_t size, size_t *bytes_written, TickType_t ticks_to_wait);
esp_err_t i2s_driver_install(i2s_port_t i2s_num, const i2s_config_t *i2s_config, int queue_size, void *i2s_queue);
esp_err_t i2s_set_pin(i2s_port_t i2s_num, const i2s_pin_config_t *pin);

#endif // _I2S_H