
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
#include "../const.h"


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage : %s <ID du programme>", argv[0]);
        exit(EXIT_FAILURE);
    }
    int noProgramme = atoi(argv[1]);
    if (noProgramme < 0 || noProgramme > NBR_PROGS)
    {
        printf("programmeID invalide");
        exit(EXIT_FAILURE);
    }

    //SEMA_KEY et SHAREDMEM_KEY definies dans const.h
    int semaID = sem_get(SEMA_KEY, NO_SEMAPHORE);
    //va chercher la sharedMem avec une perm de 0 car on ne fait que la rechercher
    int sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
    //return la sharedMem
    void *sharedMemory = sshmat(sharedMemID);
    //donne la taille de la sharedMem
    int *tailleLogique = sharedMemory;
    //pointe vers une structure contenue dans sharedMem
    Programm *tab = sharedMemory + sizeof(int);
    //lock les ressources
    sem_down0(semaID);
    //si pas de programme à cet indice là --> erreur
    if (noProgramme >= *tailleLogique)
    {
        printf("programmeID invalide");
        exit(EXIT_FAILURE);
    }
    printf("%d", tab[noProgramme].programmeID);
    printf("%s", tab[noProgramme].fichierSource);
    printf("%s", tab[noProgramme].hasError ? "true" : "false");
    printf("%d", tab[noProgramme].nombreExcecutions);
    printf("%d", tab[noProgramme].tempsExcecution);

    // relache la sharedMem
    sshmdt(sshmat(sharedMemID));
    // libère la ressource
    sem_up0(semaID);
    return EXIT_SUCCESS;
}