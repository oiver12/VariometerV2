#ifndef SOUNDMANAGER_VARIO_H
#define SOUNDMANAGER_VARIO_H

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"
#include "main.h"
#include "esp_event.h"

typedef enum
{
    constant_Deep = 0,
    no_Sound = 1,
    Biepen = 2,
} biepState_t;

typedef struct{
    biepState_t state;
    float frequency;
    int duration;
    float lowerVelocity;
    float upVelocity;
} Tonestate_t;

typedef struct{
    Tonestate_t *ToneArray;
    size_t sizeToneArray;
    int indexCurrentToneArray;
    //benötigt um zu biepen --> nur einmal ausrechnen wenn Geschwindigkiet gesetzt wird
    int frequencyNow;
    int durationNow;
    float lastBeepVelocity;
} SoundManager_t;

esp_err_t init_SoundManager(SoundManager_t *sM);
esp_err_t deinit_SoundManager(SoundManager_t *sM);
void SoundManager_calculateNewBiep(SoundManager_t *sM, float newVelocity);
esp_err_t SoundManager_playSinWave(int frequency, int duration);

#endif