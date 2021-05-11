#include "../utils_v10.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#define PERM 0666
#define SHAREDMEMSIZE sizeof(int) + 1000*sizeof(struct Programme)

int createSharedMemory(key_t key){
    return sshmget(key, SHAREDMEMSIZE, IPC_CREAT | PERM);
}

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
    //voir IPC_conf.h sem6  créer same acceskey entre programmes
    key_t keylock; // TODO initialiser les clefs dans const.h
    int noSemaphore = 1;  //TODO bonne valeur ?
    if (type == 1)
    {
        sharedMemID = createSharedMemory(keylock);
        semaID = sem_create(keylock, 1, IPC_CREAT | PERM, 0);
    }
    else if (type == 2)
    {
        sharedMemID = sshmget(keylock, SHAREDMEMSIZE, 0);
        //sharedMemID = createSharedMemory(keylock);
        sshmdelete(sharedMemID);
        semaID = sem_get(keylock, noSemaphore);
        sem_delete(semaID);
    }
    else if (type == 3)
    {
        checkCond(argc != 3, "la durée de recurrence n'est pas définie\n");
        int duration = atoi(argv[2]);
        checkNeg(duration, "la duree ne peut pas etre negative\n");
       
        //sharedMemID = createSharedMemory(keylock);
        semaID = sem_get(keylock, noSemaphore);
        //sshmat(sharedMemID);
        sem_down0(semaID);
        sleep(duration);
        sem_up0(semaID);
       //sshmdt(sshmat(sharedMemID));
    }
    else
    {
        printf("Le type est incorrect !");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}