#include "utils_v10.h"
#include "./const.h";
#include <dirent.h>
#include <stdio.h>

#define NBR_PROGS 1000

struct Programme repertoireProgs[NBR_PROGS];

void init_state(){
//initialise le repertoire avec tous les programmes
//malloc(1000 * sizeof(Programme));
struct Programme repertoireProgs[NBR_PROGS];
for (int i = 0; i < NBR_PROGS; i++)
{
    Programme programme;
    programme.programmeID = i;
    struct dirent *dir;
    // opendir() renvoie un pointeur de type DIR. 
    DIR *d = opendir("../repertoireProgs"); 
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            programme.fichierSource = dir->d_name;
        }
        closedir(d);
    }
    programme.hasError = true;
    programme.nombreExcecutions = 0;
    programme.tempsExcecution = 0;
}


}

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Usage : %s <ID du programme>", argv[0]);
        exit(EXIT_FAILURE);
    }
    int noProgramme = atoi(argv[1]);
    if(noProgramme <0 || noProgramme > NBR_PROGS){
        printf("programmeID invalide");
        exit(EXIT_FAILURE);
    }
    init_state();
    Programme *programme = repertoireProgs[noProgramme-1];
    key_t key;
    int noSemaphore;
    int semaID = sem_get(key, noSemaphore);
    //TODO init sharedMemo + attacher
    sem_down0(semaID);
    printf("%d", programme.programmeID);
    printf("%s", programme.fichierSource);
    printf("%s", programme.hasError ? "true" : "false");
    printf("%d", programme.nombreExcecutions);
    printf("%d", programme.tempsExcecution);


    //detacher sharedMemo
    sem_up0(semaID);
    return EXIT_SUCCESS;
}