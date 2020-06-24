/******************************************************************************/
/*			Application: ....			              */
/******************************************************************************/
/*									      */
/*			 programme  SERVEUR 				      */
/*									      */
/******************************************************************************/
/*									      */
/*		Auteurs :  ....						      */
/*		Date :  ....						      */
/*									      */
/******************************************************************************/	

#include<stdio.h>
#include <curses.h>

#include<sys/signal.h>
#include<sys/wait.h>
#include<stdlib.h>
#include <sys/socket.h>

#include "fon.h"     		/* Primitives de la boite a outils */

#include <dirent.h>

#define SERVICE_DEFAUT "1111"

#define BUFFER_SIZE 65536

#define PUBLIC_FOLDER_PATH "../public"

#define PARSED_GET 1
#define PARSED_PUT 2
#define PARSED_LS 3


void serveur_appli (char *service);   /* programme serveur */


/******************************************************************************/	
/*---------------- programme serveur ------------------------------*/

int main(int argc,char *argv[])
{

	char *service= SERVICE_DEFAUT; /* numero de service par defaut */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc)
 	{
   	case 1:
		  printf("defaut service = %s\n", service);
		  		  break;
 	case 2:
		  service=argv[1];
            break;

   	default :
		  printf("Usage:serveur service (nom ou port) \n");
		  exit(1);
 	}

	/* service est le service (ou numero de port) auquel sera affecte
	ce serveur*/
	
	serveur_appli(service);
}


/******************************************************************************/	
void serveur_appli(char *service)

/* Procedure correspondant au traitemnt du serveur de votre application */

{
	char buffer[BUFFER_SIZE];

	int socket = h_socket();
	struct sockaddr_in *p_address_socket = malloc(sizeof(sockaddr_in));
	adr_socket(service, IP_DEFAUT, typesock, p_address_socket); // Initialisation de la socket

	int bind = h_bind(socket, p_address_socket);

	int nb_req_att = 1; // Nombre de requêtes à écouter
	h_listen(socket, nb_req_att);
	for (int i = 0; i < nb_req_att; i++)
	{
		h_accept(socket, /* on doit mettre ici le struct sockaddr_in* du client mais comment l'obtenir ??? */);
		h_reads(socket, buffer, taille_buffer);
		
		// Lire ici dans le buffer.
		// Parser la commande lue.
		int parsed_command = parse_buffer(buffer, BUFFER_SIZE);

		// Réagir en fonction de la commande lue.
		switch (parsed_command)
		{
		case PARSED_GET:
			/* code */
			break;
		case PARSED_PUT:
			/* code */
			break;
		case PARSED_LS:
			/* code */
			break;
		
		default:
			h_writes(socket, "Erreur : commande non valide.", strlen("Erreur : commande non valide."));
			h_close(socket);
			break;
		}
	}
}

/******************************************************************************/	

/**
 * Lit le contenu du buffer et retourne la commande lue.
 * @return 1 si la commande est un get
 * @return 2 si la commande est un put
 * @return 3 si la commande est un ls
 * @return -1 sinon
 * */
int parse_buffer(char *tampon, int taille_buffer){
	return -#
}


int get_file(char* filename, int num_soc, char *buffer, int nb_octets_buffer)
{

	int nb_octets_ecrits = 0;

	if ((FILE*f=fopen(filename,"r"))==NULL)
	{
		return 0;
	}

  	while(fgets(buffer,nb_octets_buffer,f))
    {
		nb_octets_ecrits += h_writes(num_soc, buffer, nb_octets_buffer);
	}
	// write() un délimiteur de fin ?
	
	// La fermeture de la connexion avec le client se fait en dehors de cette fonction 
	return nb_octets_ecrits;
}

/**
 * Fonction appelée par le serveur lorsque celui-ci recevra de la part du client une commande "ls". Liste alors tous les fichiers contenus dans le répertoire courant.
 * */
int ls_dir(int num_soc, char *buffer, int nb_octets_buffer)
{
	int nb_octets_ecrits = 0;

	struct dirent *dir;  // Pointer for directory entry 
	DIR *d = opendir(PUBLIC_FOLDER_PATH);

	if (d == NULL)
	{
		printf("Could not open current directory." ); 
		return 0;
	}

	while ((dir = readdir(d)) != NULL) {
		printf("%s\n", dir->d_name);
		// Besoin de mettre dir->d_name dans le buffer!
		nb_octets_ecrits += h_writes(num_soc, buffer, nb_octets_buffer);
	}
	closedir(d);

	return nb_octets_ecrits;
}

