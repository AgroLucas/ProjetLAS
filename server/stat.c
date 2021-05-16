
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
    sem_down0(semaID);
    
    SharedMemoryContent* content = sshmat(sharedMemID);
    Program* tab = content->programTab;

    if (programmId >= content->logicalSize) {
        perror("ID de programme invalide\n");
        exit(EXIT_FAILURE);
    }

    Program program = content->programTab[programmId];

    printf("-----------------------------------------------\n");
    printf("Statistique pour le programme numéro %d\n", program.programId);
    printf("\t- Nom du fichier source : %s\n", program.progName);
    printf("\t- %s\n", program.hasError 
        ? "Possède actuellement une ou plusieurs erreurs à la compilation"
        : "S'est bien compilé sans aucune erreur");
    printf("\t- Nombre d'éxecutions : %d\n", program.executionNumber);
    printf("\t- Temps total d'éxecution : %d\n", program.executionTime);
    printf("-----------------------------------------------\n");

    sshmdt(sshmat(sharedMemID));
    sem_up0(semaID);
    exit(EXIT_SUCCESS);
}