#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define MAX_MESSAGE_LEN 1024
#define PORT 5055

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

typedef struct Request{
    int code;
    char identifiant[20];
    Contact c;
}Req;

typedef struct Response{
    int profile;
    Contact contact_found[5];
}Res;

typedef struct loginInfo{
    char logName[20];
    int password;
}loginInfo;

void error(char *message)
{
    perror(message);
    exit(1);
}

int MenuAdmin() {
    int choix;
    printf("\n*************************************MENU******************************************\n");
        printf("1. Ajouter un contact\n");
        printf("2. Rechercher un contact\n");
        printf("3. Supprimer un contact\n");
        printf("4. Modifier un contact\n");
        printf("5. Afficher tous les contacts\n");
        printf("6. Quitter\n");
        printf("Entrez votre choix : ");
        scanf("%d", &choix); while(getchar()!='\n');
    return choix;
}

int menuInvite() {
    int choix;
    printf("\n1. Rechercher un contact par GSM\n");
    printf("2. Afficher la liste des contacts\n");
    printf("3. Quitter\n");
    printf("Entrez votre choix : ");
    scanf("%d", &choix); while(getchar()!='\n');
    return choix;
}

void afficherContact(const Contact* c){
    if (c != NULL) {
        printf("Nom: %s %s, Email: %s, GSM: %s, Adresse: %s, %s, %s\n",
               c->nom, c->prenom, c->email, c->GSM, c->adresse.rue, c->adresse.ville, c->adresse.pays);
    } else {
        printf("Contact non trouv�.\n");
    }
}

Contact Saisir() {
    Contact c;
    printf("Donner le nom du contact : ");
    fgets(c.nom, sizeof(c.nom), stdin);// Remplace gets() ou scanf()
    c.nom[strcspn(c.nom, "\n")] = 0; // Supprime le caractère de nouvelle ligne

    printf("Donner le prenom: ");
    fgets(c.prenom, sizeof(c.prenom), stdin); //while(getchar()!='\n');
    c.prenom[strcspn(c.prenom, "\n")] = 0;

    printf("Donner l'email: ");
    fgets(c.email, sizeof(c.email), stdin); //while(getchar()!='\n');
    c.email[strcspn(c.email, "\n")] = 0;

    printf("Donner le GSM: ");
    fgets(c.GSM, sizeof(c.GSM), stdin); //while(getchar()!='\n');
    c.GSM[strcspn(c.GSM, "\n")] = 0;

    printf("\nSaisie de l'adresse:\n");
    printf("Veuillez entrer le nom de la rue: ");
    fgets(c.adresse.rue, sizeof(c.adresse.rue), stdin); //while(getchar()!='\n');
    c.adresse.rue[strcspn(c.adresse.rue, "\n")] = 0;

    printf("Veuillez entrer le nom de la ville: ");
    fgets(c.adresse.ville, sizeof(c.adresse.ville), stdin); //while(getchar()!='\n');
    c.adresse.ville[strcspn(c.adresse.ville, "\n")] = 0;

    printf("Veuillez entrer le nom du pays: ");
    fgets(c.adresse.pays, sizeof(c.adresse.pays), stdin); //while(getchar()!='\n');
    c.adresse.pays[strcspn(c.adresse.pays, "\n")] = 0;

    printf("\nContact est ajouté avec succès !\n");
    return c;
}

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        error("Failed to initialize WSAStartup...");
    #endif

    if (argc != 2)
        error("Pour executer, il faut ecrire: <file> <ipv4>\n");

    char ipAdress[17];
    strcpy(ipAdress, argv[1]);

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
        error("Erreur de création de socket\n");

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr(ipAdress);

    printf("Waiting for connection on port: %d... \n", PORT);
    if (connect(clientSocket, (struct sockaddr*) &servAddr, sizeof(servAddr)) == -1)
        error("connect() failed ");

    printf("\nWaiting for permission to communicate on port: %d... \n", PORT);

    Req req;
    Res res;
    loginInfo auth;
    int profile = -1;
    int nbAttempt = 3;

    do {
        if (nbAttempt != 3)
            printf("Erreur de l'authentification.\n\t Il vous reste %d essaie\n\n", nbAttempt);

        printf("LogName: ");
        scanf("%s", auth.logName);
        while (getchar() != '\n');
        printf("Password: ");
        scanf("%d", &auth.password);
        while (getchar() != '\n');

        send(clientSocket, (char*)&auth, sizeof(loginInfo), 0);

        if (recv(clientSocket, (char *)&profile, sizeof(profile), 0) == -1)
            error("recv() failed ");

        nbAttempt--;
    } while (profile == -1 && nbAttempt > 0);

    if (profile == -1)
        printf("\nL'authentifiacation a echoue....\n");
    else {
        if (profile == 1) {
            printf("\n----------------------Vous etes Administrateur------------------------\n");
            do
            {
                int choix = MenuAdmin();
                switch (choix) {
                    case 1:
                        req.code = 1;
                        req.c = Saisir();
                        send(clientSocket, (char*) &req, sizeof(req), 0);
                        recv(clientSocket, (char*) &res, sizeof(res), 0);
                        if (res.profile == 1) {
                            printf("ajout avec succes\n");
                        } else {
                            printf("echec d'ajout\n");
                        }
                        break;
                    case 2:
                        req.code = 2;
                        printf("Entrez le GSM du contact que vous voulez rechercher : ");
                        scanf("%s", req.c.GSM); while(getchar()!='\n');
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);

                        if(res.profile != 0)
                            afficherContact(&res.contact_found[0]);
                        else
                            printf("Le contact avec le GSM: %s n'est pas trouvé\n", req.c.GSM);

                        break;

                    case 3:
                        req.code = 3;
                        printf("Entrez le numéro de GSM à supprimer: ");
                        scanf("%s", req.c.GSM); while(getchar()!='\n');
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);
                        if (res.profile == 1) {
                            printf("Suppression avec succès\n");
                        } else {
                            printf("Échec de suppression\n");
                        }
                        break;
                    case 4:
                        req.code = 4;

                        // Get the new GSM number for modification
                        printf("Entrez le nouveau numéro de GSM à modifier : ");
                        scanf("%s", req.c.GSM); while(getchar()!='\n');

                        // Send the request to the server
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);

                        // Check if the server acknowledges the request
                        if (res.profile == 1) {
                            // Request acknowledged, now get the new contact details
                            printf("Entrez les nouvelles informations pour le contact :\n");
                            printf("---------------------------------------------\n");
                            printf("Nom : ");
                            scanf("%s", req.c.nom); while(getchar()!='\n');
                            printf("Prenom : ");
                            scanf("%s", req.c.prenom); while(getchar()!='\n');
                            printf("Email : ");
                            scanf("%s", req.c.email); while(getchar()!='\n');
                            printf("GSM : ");
                            scanf("%s", req.c.GSM); while(getchar()!='\n');
                            printf("Rue : ");
                            scanf("%s", req.c.adresse.rue); while(getchar()!='\n');
                            printf("Ville : ");
                            scanf("%s", req.c.adresse.ville); while(getchar()!='\n');
                            printf("Pays : ");
                            scanf("%s", req.c.adresse.pays); while(getchar()!='\n');

                            // Send the modified contact details to the server
                            send(clientSocket,(char*) &req, sizeof(req), 0);
                            recv(clientSocket,(char*) &res, sizeof(res), 0);

                            // Check if the modification was successful
                            if (res.profile == 1) {
                                printf("Modification effectuée avec succès.\n");
                            } else {
                                printf("Échec de la modification.\n");
                            }
                        } else {
                            printf("Échec de la demande de modification.\n");
                        }
                        break;
                    case 5:
                        req.code = 5;
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);
                        if(res.profile != 0)
                        {
                            for (int i = 0; i < res.profile; i++)
                                afficherContact(&res.contact_found[i]);
                        }
                        else
                            printf("Aucun contact trouve...\n");
                            
                        break;
                    case 6:
                        req.code = 6;
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);
                        break;
                    default:
                        printf("choix invalide 1!!");
                        break;
                }
            } while (res.profile != -2);
            
        } else {
            printf("\n-------------------------Vous etes Invite-------------------------\n");

            do
            {
                int choix = menuInvite();
                switch (choix) {
                    case 1:
                        req.code = 1;
                        printf("Entrez le GSM du contact que vous voulez rechercher : ");
                        scanf("%s", req.c.GSM); while(getchar()!='\n');
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);

                        if(res.profile != 0)
                            afficherContact(&res.contact_found[0]);
                        else
                            printf("Le contact avec le GSM: %s n'est pas trouvé\n", req.c.GSM);

                        break;
                    case 2:
                        req.code = 2;
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);

                        if(res.profile != 0)
                        {
                            for (int i = 0; i < res.profile; i++)
                                afficherContact(&res.contact_found[i]);                          
                        }
                        else
                            printf("Aucun contact trouve...\n");

                        break;
                    case 3:
                        req.code = 3;
                        send(clientSocket,(char*) &req, sizeof(req), 0);
                        recv(clientSocket,(char*) &res, sizeof(res), 0);
                        break;
                    default:
                        printf("Choix non valide.\n");
                        break;
                }
            } while (res.profile != -2);
        
        }
    }

   printf("Quitting....\n");

    #ifdef _WIN32
    WSACleanup();
    closesocket(clientSocket);
    #else
    close(clientSocket);
    #endif
    return 0;
}

