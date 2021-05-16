
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "servConst.h"
#include "../utils_v10.h"



int main(int argc, char **argv) {
    if (argc != 2) {
        perror("Usage : ./stat <ID du programme>\n");
        exit(EXIT_FAILURE);
    }
    int programmId = atoi(argv[1]);
    if (programmId < 0 || programmId > NBR_PROGS) {
        perror("ID de programme invalide\n");
        exit(EXIT_FAILURE);
    }

    int semaID = sem_get(SEMA_KEY, NO_SEMAPHORE);
    int sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
    
    SharedMemoryContent* content = sshmat(sharedMemID);
    Programm* tab = content->programTab;
    //lock les ressources
    //sem_down0(semaID);
    //si pas de programme à cet indice là --> erreur
    if (programmId >= content->logicalSize) {
        perror("ID de programme invalide\n");
        exit(EXIT_FAILURE);
    }
    printf("%d\n", (tab)[programmId].programmeID);
    printf("%s\n", (tab)[programmId].fichierSource);
    printf("%s\n", (tab)[programmId].hasError ? "true" : "false");
    printf("%d\n", (tab)[programmId].nombreExcecutions);
    printf("%d\n", (tab)[programmId].tempsExcecution);

    // relache la sharedMem
    sshmdt(sshmat(sharedMemID));
    // libère la ressource
   //sem_up0(semaID);
    exit(EXIT_SUCCESS);
}