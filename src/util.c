#include <string.h>
#include <stdlib.h>

#include "util.h"

int validateNumber(const char* number)
{
    for (int i = 0; i < strlen(number); i++) {
        if (number[i] < '0' || number[i] > '9') {
            return 0;
        }
    }
    return 1;
}

int randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

char *mapDataPath(char* file)
{
    char* programDir;
    if ((programDir = getenv("PROGRAM_DIR")) == NULL) {
        programDir = "..";
    }
    char* dataPath = malloc(strlen(programDir) + strlen(file) + 1);
    strcpy(dataPath, programDir);
    strcat(dataPath, "/");
    strcat(dataPath, file);
    return dataPath;
}
