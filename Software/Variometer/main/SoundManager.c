#include "SoundManager.h"
#include "driver/i2s.h"

#define EXAMPLE_PDM_TX_FREQ_HZ          44100           // I2S PDM TX frequency
#define SAMPLE_RATE     (36000)
#define I2S_NUM         (0)

#define I2S_BCLK 11
#define I2S_LRCLK 12
#define I2S_DIN 10
#define AUDIO_EN 21
#define AUDIO_GAIN 47

esp_err_t start_SoundManager()
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRCLK,
        .data_out_num = I2S_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    return ESP_OK;
}