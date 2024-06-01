#include "prototype.h"

/*----------------------------------- AUTHENTIFICATION ----------------------------------*/

int authentification(char *comptesFile, loginInfo clientLogin)
{
    loginInfo temp; char profile[10];
    FILE *file = fopen(comptesFile, "r");

    if(file != NULL)
    {
        while (!feof(file))
        {
            if(fscanf(file, "%s %d %s", temp.logName, &temp.password, profile) != -1)
            {
                if(strcmp(clientLogin.logName, temp.logName) == 0 && clientLogin.password == temp.password)
                    return (strcmp(profile, "Admin") == 0)? 1 : 2;
            }
        }
          
    }
    else
        printf("File not found. Server crushed...\n");

    return -1;
}

/*----------------------------------- AJOUTER CONTACT ----------------------------------*/

 int Ajouter(char *file, Contact c) {
    int resultat = 0;
    FILE *fp = fopen(file, "a+");
    if (fp != NULL) {
        // Utilisation de # comme s�parateur entre les champs
        fprintf(fp, "%s#%s#%s#%s#%s#%s#%s\n", c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays);
        fclose(fp);
        resultat = 1;
    } else {
        printf("Erreur lors de l'ouverture du fichier !!\n");
        resultat = 0;
    }
    return resultat;
}

/*----------------------------------- SUPPRIMER CONTACT ----------------------------------*/

int supprimer_Contact(char *file, char *num_GSM) {
    Contact c;
    int resultat = 0; // Pas trouv� initialement
    FILE *f_in = fopen(file, "r");
    FILE *f_out = fopen("temp.txt", "w+");
    if (f_in != NULL && f_out != NULL) {
        while (fscanf(f_in, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#\n]\n",
                      c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays) != EOF) {
            if (strcmp(c.GSM, num_GSM) != 0) {
                // Si le contact n'est pas celui � supprimer, on l'�crit dans le fichier temporaire
                fprintf(f_out, "%s#%s#%s#%s#%s#%s#%s\n",
                        c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays);
            } else {
                // Contact trouv� et donc non r��crit (supprim�)
                resultat = 1;
            }
        }

        fclose(f_in);
        fclose(f_out);
        // Remplacer l'ancien fichier par le nouveau (sans le contact supprim�)
        remove(file);
        rename("temp.txt", file);
    } else {
        printf("Fichier introuvable !\n");
    }
    return resultat;
}

/*----------------------------------- MODIFIER CONTACT ----------------------------------*/

int Modifier_contact(char *file, char *gsm, Contact c1) {
    int resultat = 0;  //gsm not foundd
    FILE *F_in = fopen(file, "r");
    FILE *F_out = fopen("tmp.txt", "w");

    if (F_in == NULL || F_out == NULL) {
        printf("Erreur lors de l'ouverture des fichiers !");
        return resultat;
    }
    Contact c;
    while (!feof(F_in)) {
        if(fscanf(F_in, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#\n]\n", c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays) != -1)
        {
            if (strcmp(gsm, c.GSM) == 0) {
                fprintf(F_out, "%s#%s#%s#%s#%s#%s#%s\n", c1.nom, c1.prenom, c1.email, c1.GSM, c1.adresse.rue, c1.adresse.ville, c1.adresse.pays);
                resultat = 1; //gsm found and updated
                printf("\nModification avec succ�s !\n");
            } else {
                fprintf(F_out, "%s#%s#%s#%s#%s#%s#%s\n", c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays);
            }
        }
    }

    fclose(F_out);
    fclose(F_in);
    remove(file);
    rename("tmp.txt", file);
    return resultat;
}

/*----------------------------------- EXISTENCE D'UN CONTACT ----------------------------------*/

int contactExiste(char *fileName, const char *GSM)
{
    Contact c;
    FILE *file = fopen(fileName, "r");

    if(file != NULL)
    {
        while (!feof(file))
        {
            if(fscanf(file, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#\n]\n", c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays) != -1)
            {
                if(strcmp(GSM, c.GSM) == 0)
                {
                    printf("Trouve\n");
                    fclose(file);
                    return 1; //trouvé
                }
            }
        }
        fclose(file);     
    }
    return 0;
}

/*----------------------------------- RECHERCHE D'UN CONTACT ----------------------------------*/
Contact* rechercherContact(const char* GSMRecherche, const char* fichier) {
    FILE *fp = fopen(fichier, "r");
    if (fp == NULL) {
        printf("Erreur lors de l'ouverture du fichier\n");
        return NULL;
    }

    Contact* cFound = NULL;
    Contact c;
    char ligne[256];
    while (fgets(ligne, sizeof(ligne), fp) != NULL) {
        if (sscanf(ligne, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]",
                   c.nom, c.prenom, c.email, c.GSM, c.adresse.rue, c.adresse.ville, c.adresse.pays) == 7) {
            if (strcmp(c.GSM, GSMRecherche) == 0) { // Comparaison bas�e sur le GSM

                cFound = (Contact*)malloc(sizeof(Contact));
                if (cFound != NULL) {
                    *cFound = c;
                    break;
                } else {
                    printf("Erreur d'allocation de memoire.\n");
                    fclose(fp);
                    return NULL;
                }
            }
        }
    }

    fclose(fp);
    return cFound;
}

/*----------------------------------- AFFICHER TOUS LES CONTACTS ----------------------------------*/

Contact* lireContacts(const char* fichier, int* nbContacts) {
    FILE *fp = fopen(fichier, "r");
    if (fp == NULL) {
        printf("Impossible d'ouvrir le fichier\n");
        *nbContacts = 0; // Aucun contact lu
        return NULL;
    }

    int capacite = 20; // Capacit� initiale du tableau
    Contact* contacts = (Contact*)malloc(capacite * sizeof(Contact));
    if (contacts == NULL) {
        fclose(fp);
        *nbContacts = 0;
        return NULL; // �chec de l'allocation de m�moire
    }

    *nbContacts = 0;
    char ligne[256];

    while (fgets(ligne, sizeof(ligne), fp) != NULL) {
        if (*nbContacts >= capacite) {
            capacite *= 2; // Double la capacit�
            Contact* nouveauxContacts = (Contact*)realloc(contacts, capacite * sizeof(Contact));
            if (nouveauxContacts == NULL) {
                free(contacts);
                fclose(fp);
                *nbContacts = 0;
                return NULL;
            }
            contacts = nouveauxContacts;
        }

        if (sscanf(ligne, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]",
                   contacts[*nbContacts].nom, contacts[*nbContacts].prenom, contacts[*nbContacts].email, contacts[*nbContacts].GSM,
                   contacts[*nbContacts].adresse.rue, contacts[*nbContacts].adresse.ville, contacts[*nbContacts].adresse.pays) == 7) {
            
            (*nbContacts)++;
        }
    }

    fclose(fp);
    return contacts;
}
