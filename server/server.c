/*
******* HOW TO COMPILE ON WINDOWS **********

gcc -Wall -Wextra <fileName.c> -o <executableFileName> -lws2_32
*/
#include "prototype.h"

#define PENDING_QUEUE_MAXLEN 5
#define MAX_CLIENT_HANDLED 2
#define PORT 5055
#define CONTACT_FILE "contacts.txt"
#define COMPTES_FILE "comptes.txt"

/*-------------------------- SEMAPHORE --------------------------*/

//semaphore for count down by MAX_CLIENT_HANDLED
sem_t sem_nbConnxClientDispo;
sem_t lock_sockID;

/*-------------------------- PROTOTYPE --------------------------*/

void error(char *message)
{
    perror(message);
    exit(1);
}

void* handleClientRequest(void *clientSocket);
void adminRequest(int sockfd, char logName[20]);
void inviteRequest(int sockfd, char logName[20]);

/*--------------------------- MAIN ----------------------------*/
int main()
{
    /*--------------- CONFIG FOR WINDOWS-----------------*/
    #ifdef _WIN32
        WSADATA  wsa;
        if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
            error("Failed initializing WSAStartup...");

    #endif
    
    printf("=============================== **SERVEUR** ==================================\n\n");
    
    /*-- Init semaphore and mutex to protect the access to clientSocketID --*/
    sem_init(&sem_nbConnxClientDispo, 0, MAX_CLIENT_HANDLED);
    sem_init(&lock_sockID, 0, 1);

    /*-- creating socket --*/
    int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(servSock == -1)
        error("socket() failed ");

    //binding address to the socket
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servAddr.sin_addr.s_addr = inet_addr("192.168.137.1");


    if(bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
        error("bind() failed ");

    printf("\t\tLa Socket est maintenant ouverte en mode TCP/IP sur le port : %d\n\n", PORT);

    //listening to the port
    if(listen(servSock, PENDING_QUEUE_MAXLEN) == -1)
        error("listen() failed ");

    /*-- CREATING A THREAD FOR EACH NEW CLIENT --*/
    pthread_t newClient;
    int clientSocket;

    /*---------------------- MAKE SERVER TO BE ALERT(LISTEN) ENDLESSLY -------------------*/

    while (1)
    {
        printf("---------------> Nouvelle demande de connexion au serveur <---------------\n\n"); 
        //Decrement sem value and block if it's 0
            sem_wait(&sem_nbConnxClientDispo);
        //client address
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        sem_wait(&lock_sockID);
            clientSocket = accept(servSock, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if(clientSocket == -1)
        {
            printf("Creating communication with client (%s) failed.\n", inet_ntoa(clientAddr.sin_addr));
            perror("error ");
            sem_post(&lock_sockID);
        }
        else
        {   
            /*--------------- CREATING THREAD FOR NEW CLIENT LOGGED IN ----------------------*/
            if(pthread_create(&newClient, NULL, handleClientRequest, (void *)&clientSocket) != 0)
            {
                printf("Estabilishing communication failed\n"); perror("Error ");
                sem_post(&sem_nbConnxClientDispo);
            }  
                    
        }         
    }

    /*--------------------- CLOSING AND FREE RESSOURCES --------------------*/
    //close semaphore
    sem_destroy(&sem_nbConnxClientDispo);
    sem_destroy(&lock_sockID);
    //closing the socket and free memory
    #ifdef _WIN32
        WSACleanup();
        closesocket(servSock);
    #else
        close(servSock);
    #endif

    return 0;
}

/*-- Routine for new client connexion --*/
void* handleClientRequest(void *clientSocket)
{
    int profile = -1, nbAttempt = 3;
    loginInfo clientAuth;
    int sock = *(int*)clientSocket;
    sem_post(&lock_sockID); 
    
    printf("+++++++++++++++++++ SERVEUR PRET A COMMUNIQUER ++++++++++++++++++++\n\n");

    /*------------------ AUTHENTIFICATION -------------------------*/
    do
    {
        //je vais recevoir les login/mdp des clients ici (recv)
        if(recv(sock, (char*)&clientAuth, sizeof(loginInfo), 0) == -1)
        {
            perror("Receiving Authentification failed ");
            break;
        }
            //je dois envoyer un message au client ici ou le profile pour dire au socket client l'erreur
        else
        {
            printf("--->%s : Demande d'authentification\n", clientAuth.logName);
            profile = authentification(COMPTES_FILE, clientAuth);
            printf("prifile: %d\n", profile);
            send(sock, (char *)&profile, sizeof(profile), 0);            
            nbAttempt--;
        }
    } while (profile == -1 && nbAttempt > 0);
      

    /*------------------ PRINTING SPECIFIC MENU FOR EACH PROFILE ------------------*/
    if(profile == 1) //Client admin
        adminRequest(sock, clientAuth.logName);
    else if(profile == 2) // client host(not admin)
        inviteRequest(sock, clientAuth.logName);
    
    //sem_post tokony ataoko aty refa mi-quitte ny client fa mi-fermer ny socket-ny (deconnecte)
    sem_post(&sem_nbConnxClientDispo);

    if(profile != -1)
        printf("\nClient %s loged out\n", clientAuth.logName);

    printf("\n----------SOCKET DISCONNECTED\n");
    // pthread_exit(NULL);
    return NULL;
}

void adminRequest(int sockfd, char logName[20])
{
    Contact *temp;
    int nbContact = 0;
    Req req; //le requête pourrait être une struct
    Res res; //la reponse pourrait être une struct
    printf("\nTrouve. Profile du client: Admin.\n");
    while(1)
    {
        memset(&res, 0, sizeof(Res));
        memset(&req, 0, sizeof(Req));
        if(recv(sockfd, (char *)&req, sizeof(Req), 0) != -1)
        {
            switch (req.code)
            {
                case 1:
                    printf("->%s (Admin) : Ajout contact.\n", logName);
                    res.status = Ajouter(CONTACT_FILE, req.c); // res.status = (succes)? 1 : 0;                    
                    break;

                case 2:
                    printf("->%s (Admin) : Rechercher un contact.\n", logName);
                    //fction de recherche de contact
                    temp = rechercherContact(req.c.GSM, CONTACT_FILE);              
                    res.status = (temp == NULL)? 0 : 1; //0: non trouvé, 1: trouvé
                    if(res.status != 0)
                        res.contact_found[0] = *temp;

                    break;
                case 3:
                    printf("->%s (Admin) : Supprimer un contact.\n", logName);
                    printf("Contact a supprimer: %s\n", req.c.GSM);
                    res.status = supprimer_Contact(CONTACT_FILE, req.c.GSM); // res.status = (succes)? 1 : 0; //0 si non trouvé
                    break;

                case 4:
                    printf("->%s (Admin) : Modifier un contact.\n", logName);

                    res.status = contactExiste(CONTACT_FILE, req.c.GSM);
                    send(sockfd, (char *)&res, sizeof(Res), 0);

                    if(recv(sockfd, (char*)&req, sizeof(Req), 0) == -1)
                    {
                        res.status = 0;
                        printf("Erreur de reception des donnees de modification.\n");
                    }
                    else
                    {
                        res.status = Modifier_contact(CONTACT_FILE, req.c.GSM, req.c);
                        if (res.status == 1) {
                            printf("Modification avec succès.\n");
                        } else {
                            printf("Échec de la modification.\n");
                        }
                    }       
                    break;
                
                case 5:
                    printf("->%s (Admin) : Afficher tous les contacts.\n", logName);
                    //fction d'affichage de tous les contacts
                    temp = lireContacts(CONTACT_FILE, &nbContact);
                    res.status = (temp == NULL)? 0 : nbContact;
                    if (res.status != 0)
                    {
                        for (int i = 0; i < nbContact; i++)
                            res.contact_found[i] = temp[i];                  
                    }
                    break;
                
                case 6:
                    printf("->%s (Admin) : Quitter(Se deconnecter).\n", logName);
                    res.status = -2;
                    break;
                
                default:
                    printf("Choix invalide. Erreur partie client.\n");
                    res.status = -1;
                    break;
            }
        }
        else
            res.status = -1; //erreur server. Veuillez reessayer.
        
        send(sockfd, (char *)&res, sizeof(res), 0);
        //If the client want to quit
        if(req.code == 6)
            break;

    }
}

void inviteRequest(int sockfd, char logName[20])
{
    int nbContact;
    Contact *temp;
    Req req; //le requête pourrait être une struct
    Res res; //la reponse pourrait être une struct
    printf("\nTrouve. Profile du client: Invite.\n");
    while(1)
    {
        if(recv(sockfd, (char *)&req, sizeof(Req), 0) != -1)
        {
            switch (req.code)
            {
                case 1:
                    printf("->%s (Invite) : Rechercher un contact.\n", logName);
                    //fction de recherche de contact
                    temp = rechercherContact(req.c.GSM, CONTACT_FILE);              
                    res.status = (temp == NULL)? 0 : 1; //0: non trouvé, 1: trouvé
                    if(res.status != 0)
                        res.contact_found[0] = *temp;
    
                    break;
                
                case 2:
                    printf("->%s (Invite) : Afficher tous les contacts.\n", logName);
                    //fction d'affichage de tous les contacts
                    temp = lireContacts(CONTACT_FILE, &nbContact);
                    res.status = (temp == NULL)? 0 : nbContact;
                    if (res.status != 0)
                    {
                        for (int i = 0; i < nbContact; i++)
                            res.contact_found[i] = temp[i];                  
                        
                    }
                    
                    break;
                
                case 3:
                    printf("->%s(invite) : Quitter(Se deconnecter).\n", logName);
                    res.status = -2;
                    break;
                
                default:
                    printf("Choix invalide. Erreur partie client.\n");
                    res.status = -1;
                    break;
            }
        }
        // else
        //     res.status = -1;

        send(sockfd, (char *)&res, sizeof(res), 0);

        //if the client want to quit
        if(res.status == -2)
            break;
         
    }
}

