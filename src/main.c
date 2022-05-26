#include <stdlib.h>
#include <time.h>

#include "service.h"
#include "gui.h"

int main(int argc, char* argv[], char* envp[])
{
    srand(time(NULL));
    initPizzaService();
    int result = app_main(argc, argv);
    destroyPizzaService();
    return result;
}
