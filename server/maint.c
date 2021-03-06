#include "../utils_v10.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#include "servConst.h"


int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage : %s type [opt]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int type = atoi(argv[1]);
    int sharedMemID;
    int semaID;
    Program programTab[NBR_PROGS];
    SharedMemoryContent sharedMemoryContent = {0, *programTab};


    switch (type) {
        case 1: //crée les ressources
            sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, IPC_CREAT | PERM);
            SharedMemoryContent* content = sshmat(sharedMemID);
            content = &sharedMemoryContent;
            semaID = sem_create(SEMA_KEY, 1, IPC_CREAT | PERM, 1);
            break;

        case 2: //détruit les ressources
            sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
            sshmdelete(sharedMemID);
            semaID = sem_get(SEMA_KEY, NO_SEMAPHORE);
            sem_delete(semaID);
            break;

        case 3: // réserve la sharedMem pour une certaine durée
            checkCond(argc != 3, "la durée de recurrence n'est pas définie\n");
            int duration = atoi(argv[2]);
            checkCond(duration <= 0, "la duree ne peut pas etre negative ou nulle\n");
            semaID = sem_get(SEMA_KEY, NO_SEMAPHORE);
            sem_down0(semaID);
            sleep(duration);
            sem_up0(semaID);
            break;

        default:
            printf("Le type est incorrect !");
            exit(EXIT_FAILURE);
        //break;
    }
    return EXIT_SUCCESS;
}