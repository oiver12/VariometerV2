#ifndef __SOUNDMANAGER_H
#define __SOUNDMANAGER_H

#include <stdio.h>

typedef struct
{
    int frequency;
    int duration;
} SoundManager_Tone_t;

void SoundManager_setVolume(int volume);

void SoundManager_playToneArray(SoundManager_Tone_t *toneArray, size_t sizeToneArray);

#endif