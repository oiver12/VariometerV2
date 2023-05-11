#include "SDCard.h"
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

void GetFlights(int startIndex, FlightFile_t flightFile[6], uint16_t* foundAfterStart)
{
    DIR *dir;
    struct dirent *ent;
    char *extension = ".IGC";

    if((dir = opendir("IGC")) != NULL)
    {
        int index = 0;
        int tookenFiles = 0;
        while((ent = readdir(dir)) != NULL)
        {
            char *fileName = ent->d_name;
            int len = strlen(fileName);
            if(len > 4 && strcmp(fileName + len - 4, extension) == 0)
            {
                index++;
                if(index < startIndex)
                    continue;
                *foundAfterStart += 1;
                strcpy(flightFile[tookenFiles].fileName, fileName);
                tookenFiles++;
                struct stat file_stat;
                if(stat(fileName, &file_stat) == 0)
                {
                    flightFile[tookenFiles].timeStamp = file_stat.st_atime;
                }
            }
            if(tookenFiles >= 6)
                break;
        }
        closedir(dir);
    }
    else
    {
        printf("Cant open directory\n");
    }
}