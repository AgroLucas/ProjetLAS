#include "utils_v10.h"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage : %s type [opt]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int type = atoi(argv[1]);
    int sharedMemID;
    int semaID;
    key_t keylock;
    int noSemaphore;  //TODO initialiser
    if (type == 1)
    {
        shs = creercharedmemory(size(int) sizetab, tab[]tab)
        sharedMemID = sshmget(keylock, sizeof(int), 1);
        semaID = sem_get(keylock, noSemaphore);
    }
    else if (type == 2)
    {
        sharedMemID = sshmget(keylock, sizeof(int), 1);
        sshmdelete(sharedMemID);
        semaID = sem_get(keylock, noSemaphore);
        sem_delete(semaID);
    }
    else if (type == 3)
    {
        checkCond(argc != 3, "la durée de recurrence n'est pas définie\n");
        int duration = atoi(argv[2]);
        checkNeg(duration, "la duree ne peut pas etre negative\n");
        sharedMemID = sshmget(keylock, sizeof(int), 1);
        semaID = sem_get(keylock, noSemaphore);
        sshmat(sharedMemID);
        sem_down0(semaID);
        sleep(duration);
        sem_up0(semaID);
        //est-ce possible de faire ça --> erreur ????? TODO
        sshmdt(sshmat(sharedMemID));
    }
    else
    {
        printf("Le type est incorrect !");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}