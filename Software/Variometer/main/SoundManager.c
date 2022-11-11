#include "SoundManager.h"
#include "driver/i2s.h"
#include <math.h>
#include <string.h>
#include <driver/gpio.h>

#define BUFF_SIZE   2048
#define PDM_TX_FREQ_HZ          44100           // I2S PDM TX frequency
#define I2S_NUM         (0)
#define VELOCITY_THRESHOLD 0.2

#define I2S_BCLK 11
#define I2S_LRCLK 12
#define I2S_DIN 10
#define AUDIO_EN 21
#define AUDIO_GAIN 47

#define WAVE_AMPTITUDE          (30000.0)        // 1~32767
#define SINE_WAVE_LEN(tone)     (uint32_t)((PDM_TX_FREQ_HZ / (float)tone) + 0.5) // The sample point number per sine wave to generate the tone
#define BYTE_NUM_EVERY_TONE(duration)     (duration * PDM_TX_FREQ_HZ / 1000)

int16_t *buffer;
int16_t bufferZero[200];

void SoundManager_Task(void *parameters)
{
    //TODO: evtl. volatile
    SoundManager_t *sM = (SoundManager_t *)parameters;
    float lastFreqeuncy = sM->frequencyNow-1;
    size_t w_bytes = 0;
    int tone_point = 0;
    while(true)
    {
        if(sM->ToneArray[sM->indexCurrentToneArray].state == no_Sound)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            continue;
        }
        if(lastFreqeuncy != sM->frequencyNow)
        {
            tone_point = SINE_WAVE_LEN(sM->frequencyNow);
            /* Generate the tone buffer */
            for (int i = 0; i < tone_point; i++) {
                buffer[i] =  (int16_t)((sin(2 * (float)i * M_PI / tone_point)) * WAVE_AMPTITUDE);
            }
            lastFreqeuncy = sM->frequencyNow;
        }
        w_bytes = 0;
        //evtl. nicht duration / 2, weil nur ein Channel?. Zeit stimmt auf jeden Fall so.
        for (int tot_bytes = 0; tot_bytes < BYTE_NUM_EVERY_TONE(sM->durationNow); tot_bytes += w_bytes) 
        {
            if(i2s_write(I2S_NUM, buffer, tone_point * sizeof(int16_t), &w_bytes, 100 / portTICK_PERIOD_MS) != ESP_OK)
            {
                printf("Write Task: i2s write failed\n");
            }
        }
        //printf("Time: %lli. Should be: %i\n", ((esp_timer_get_time() - lastTime)/1000), sM->durationNow/2);
        if(sM->ToneArray[sM->indexCurrentToneArray].state != constant_Deep)
        {
            for (int tot_bytes = 0; tot_bytes < BYTE_NUM_EVERY_TONE(sM->durationNow); tot_bytes += w_bytes) 
            {
                if(i2s_write(I2S_NUM, bufferZero, sizeof(bufferZero), &w_bytes, 100 / portTICK_PERIOD_MS) != ESP_OK)
                {
                    printf("Write Task: i2s write failed\n");
                }
            }
        }
    }
    vTaskDelete(NULL);
}

void SoundManager_calculateNewBiep(SoundManager_t *sM, float newVelocity)
{
    //in einer For schleife herausfinden, zwischen welchen Punkten die Geschwindigkeit ist
  for (size_t i = 0; i < sM->sizeToneArray; i++)
  {
    if(newVelocity >= sM->ToneArray[i].lowerVelocity && newVelocity <= sM->ToneArray[i].upVelocity)
    {
        volatile float difference = sM->lastBeepVelocity - newVelocity;
        //Wenn Differenz Geschwindigkeit kliener als Thershold ist, dann nicht neuer Ton ausgeben, ausser man geht von nicht piepsen in piepsen oder umgekehrt
        if(fabs(difference) < VELOCITY_THRESHOLD && !(sM->indexCurrentToneArray > 1 && i == 1) && !(sM->indexCurrentToneArray  <= 1 && i > 1))
        {
            return;
        }
        sM->lastBeepVelocity = newVelocity;
        sM->indexCurrentToneArray = i;
        //wenn letzter Ton, dann einfach Geschwindigkeit nehemn
        if((sM->sizeToneArray - 1) == i || i == 0)
        {
            sM->frequencyNow = sM->ToneArray[i].frequency;
            sM->durationNow = sM->ToneArray[i].duration;
        }
        //sonst linear Interplorieren
        else
        {
            //linear zwischen zwei States machen
            float factor = (newVelocity - sM->ToneArray[i].lowerVelocity)/(sM->ToneArray[i].upVelocity-sM->ToneArray[i].lowerVelocity);
            sM->frequencyNow = factor * (sM->ToneArray[i+1].frequency - sM->ToneArray[i].frequency) + sM->ToneArray[i].frequency;
            sM->durationNow = factor * (sM->ToneArray[i+1].duration - sM->ToneArray[i].duration) + sM->ToneArray[i].duration;
        }
    }
  }
}

esp_err_t init_SoundManager(SoundManager_t *sM)
{
    gpio_config_t enableGPIOOutput = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << AUDIO_EN,
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    gpio_config(&enableGPIOOutput);
    gpio_set_level(AUDIO_EN, 1);

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = PDM_TX_FREQ_HZ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 6,
        .dma_buf_len = 240,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                               //Interrupt level 1
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRCLK,
        .data_out_num = I2S_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));
    buffer = (int16_t *)calloc(1, BUFF_SIZE);
    assert(buffer);
    
    sM->ToneArray = calloc(6, sizeof(Tonestate_t));
    assert(sM->ToneArray);
    sM->sizeToneArray = 6;
    Tonestate_t stateBeep[6] = {
        {.state = constant_Deep, .frequency = 250, .duration = 200, .lowerVelocity = -999, .upVelocity = -2.5f},
        {.state = no_Sound, .frequency = 0, .duration = 0, .lowerVelocity = -2.5f, .upVelocity = 0.1f},
        {.state = Biepen, .frequency = 700, .duration = 750, .lowerVelocity = 0.1f, .upVelocity = 1.5f},
        {.state = Biepen, .frequency = 850, .duration = 540, .lowerVelocity = 1.5f, .upVelocity = 3.5f},
        {.state = Biepen, .frequency = 1000, .duration = 300, .lowerVelocity = 3.5f, .upVelocity = 10},
        {.state = Biepen, .frequency = 1600, .duration = 90, .lowerVelocity = 10, .upVelocity = 999}
    };
    memcpy(sM->ToneArray, stateBeep, sizeof(stateBeep));
    sM->indexCurrentToneArray = 1;
    xTaskCreate(SoundManager_Task, "SoundManager_Task", 2 * 1024, sM, 7, NULL);
    return ESP_OK;
}

esp_err_t deinit_SoundManager(SoundManager_t *sM)
{
    gpio_set_level(AUDIO_EN, 0);
    free(sM->ToneArray);
    free(buffer);
    return ESP_OK;
}

esp_err_t SoundManager_playSinWave(int frequency, int duration)
{
    size_t w_bytes = 0;
    int tone_point = SINE_WAVE_LEN(frequency);
    /* Generate the tone buffer */
    for (int i = 0; i < tone_point; i++) {
        buffer[i] =  (int16_t)((sin(2 * (float)i * M_PI / tone_point)) * WAVE_AMPTITUDE);
    }
    
    for (int tot_bytes = 0; tot_bytes < BYTE_NUM_EVERY_TONE(duration*2); tot_bytes += w_bytes) {
        if(i2s_write(I2S_NUM, buffer, tone_point * sizeof(int16_t), &w_bytes, 1000) != ESP_OK)
        {
            printf("Write Task: i2s write failed\n");
        }
    }
    return ESP_OK;
}