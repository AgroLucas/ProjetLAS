#include "utils_v10.h"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage : %s type [opt]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int type = atoi(argv[1]);
    key_t keylock;
    if (type == 1)
    {
        //TODO
    }
    else if (type == 2)
    {
        //TODO
    }
    else if (type == 3)
    {
        //TODO
        checkCond(argc != 3, "la durée de recurrence n'est pas définie\n");
        int duration = atoi(argv[2]);
        checkNeg(duration, "la duree ne peut pas etre negative\n");
        sleep(duration);
    }
    else
    {
        fprintf(stderr, "%s\n", "Le type est incorrect !");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}