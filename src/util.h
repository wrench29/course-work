#ifndef UTIL_H_
#define UTIL_H_

#define CmpStr(a, b) (strcmp(a, b) == 0)

int validateNumber(const char* number);
int randomInt(int min, int max);
char *mapDataPath(char* file);

#endif
