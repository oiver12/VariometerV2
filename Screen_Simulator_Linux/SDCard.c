#include "SDCard.h"
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

FILE *openFile_sdcard(const char *directory, const char *fileName, const char *openMethod)
{
    char buffer[100];
    buffer[0] = '\0';
    if (directory != NULL)
    {
        if (directory[0] == '/')
            strcat(buffer, directory + 1);
        else
            strcat(buffer, directory);
    }
    strcat(buffer, "/");
    strcat(buffer, fileName);
    return fopen(buffer, openMethod);
}

void GetFlights(int startIndex, widget_Settings_linkedList *flightListHead)
{
    DIR *dir;
    struct dirent *ent;
    char *extension = ".igc";

    if ((dir = opendir("IGCFlights")) != NULL)
    {
        int index = 0;
        int tookenFiles = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            char *fileName = ent->d_name;
            int len = strlen(fileName);
            if (len > 4 && strcmp(fileName + len - 4, extension) == 0)
            {
                index++;
                if (index < startIndex)
                    continue;
                if (flightListHead == NULL)
                {
                    printf("Couldn't find next Flight\n");
                    return;
                }
                strcpy(((FlightList_Widget_Data *)flightListHead->data.extr_data)->FlightName, fileName);
                flightListHead = flightListHead->next;
                tookenFiles++;
            }
            if (tookenFiles >= 6)
                break;
        }
        closedir(dir);
        if (tookenFiles < 6 && flightListHead)
        {
            ((FlightList_Widget_Data *)flightListHead->data.extr_data)->FlightName[0] = '\0';
        }
    }
    else
    {
        printf("Cant open directory\n");
    }
}