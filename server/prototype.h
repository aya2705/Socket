#ifndef __PROTOTYPE_H__
#define __PROTOTYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

//derective preprocesseur
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

/*-------------- STRUCTURE CONTACT ------------*/
typedef struct {
    char rue[30];
    char ville[30];
    char pays[30];
} Adresse;

typedef struct {
    char nom[20];
    char prenom[20];
    char email[100];
    char GSM[30];
    Adresse adresse;
} Contact;

/*-------------- STRUCTURE FOR REQUEST/RESPONSE HANDLING ------------*/
typedef struct Request{
    int code;
    char identifiant[20];
    Contact c;
}Req;

typedef struct Response{
    int status;
    Contact contact_found[5];
}Res;

typedef struct loginInfo{ //pour envoyer qu'une seule fois les logins de l'utilisateur
    char logName[20];
    int password;
}loginInfo;

/*------------------------- PROTOTYPES --------------------------*/
int authentification(char *comptesFile, loginInfo clientLogin);
int Ajouter(char *file, Contact c);
int supprimer_Contact(char *file,char *num_GSM);
int Modifier_contact(char *file, char *gsm, Contact c1);
Contact* rechercherContact(const char* GSMRecherche, const char* fichier);
Contact* lireContacts(const char* fichier, int* nbContacts);
int contactExiste(char *fileName, const char *GSM);

#endif