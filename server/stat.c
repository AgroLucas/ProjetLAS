
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utils_v10.h"
#include "../const.h"

#define NBR_PROGS 1000

#define PERM 0666
//TODO mettre un fichier existant
#define PATHFILE "test.txt"
//TODO trouver la taille exacte de la memoire partagée
#define SHAREDMEMSIZE 42

//tableau contenant structure Programm
//struct Programm *repertoireProgs = createArrayOfProgramms(NBR_PROGS);

struct Programm *createArrayOfProgramms(int tabSize)
{
    struct Programm *tab = (struct Programm *)smalloc(NBR_PROGS * sizeof(struct Programme));
    for (int i = 0; i < NBR_PROGS; i++)
    {
        //TODO
        initializeStructure(&tab[i], &tab[i].programmeID, &tab[i].fichierSource, &tab[i].hasError, &tab[i].nombreExcecutions, &tab[i].tempsExcecution);
    }

    //initializeStructure(&tab[i], &tab[i].programmeID, &tab[i].fichierSource, &tab[i].hasError, &tab[i].nombreExcecutions, &tab[i].tempsExcecution);
}


static void initializeStructure(struct Programme *prog, const int *programmeID,
                                const char **fichierSource, const bool *hasError,
                                const int *nombreExcecutions, const int *tempsExcecution)
{
    strcpy(prog->programmeID, programmeID);
    strcpy(prog->fichierSource, fichierSource);
    strcpy(prog->hasError, hasError);
    strcpy(prog->nombreExcecutions, nombreExcecutions);
    strcpy(prog->tempsExcecution, tempsExcecution);
}


int createSharedMemory(key_t key)
{
    return sshmget(key, SHAREDMEMSIZE, IPC_CREAT | PERM);
}



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
    
    struct Programm *repertoireProgs = createArrayOfProgramms(NBR_PROGS);
    //TODO
    Programm programme = repertoireProgs[noProgramme - 1];
    key_t key;
    int noSemaphore = 1;
    int semaID = sem_get(key, noSemaphore);

    int sharedMemID = createSharedMemory(key);
    sshmat(sharedMemID);

    sem_down0(semaID);
    printf("%d", programme.programmeID);
    printf("%s", programme.fichierSource);
    printf("%s", programme.hasError ? "true" : "false");
    printf("%d", programme.nombreExcecutions);
    printf("%d", programme.tempsExcecution);

    sshmdt(sshmat(sharedMemID));

    sem_up0(semaID);
    return EXIT_SUCCESS;
}