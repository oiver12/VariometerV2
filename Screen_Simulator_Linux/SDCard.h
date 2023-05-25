#ifndef SDCARD_VARIO_H
#define SDCARD_VARIO_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>
#include "Screen/SettingsManager.h"

#define MAX_FILENAME 100

FILE *openFile_sdcard(const char *directory, const char *fileName, const char *openMethod);
// foundAfterStart stops after 6
void GetFlights(int startIndex, widget_Settings_linkedList *flightListHead);

#endif