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

#include "fon.h"     		/* Primitives de la boite a outils */

#define SERVICE_DEFAUT "1111"

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
	}
}

/******************************************************************************/	

int get_file(char* filename, int num_soc, char *buffer, int nb_octets_buffer)
{

	int nb_octets_ecrits = 0;

	if ((FILE*f=fopen(filename,"r"))==NULL)
	{
		return 0;
	}

  	while(fgets(buffer,100,f))
    {
		nb_octets_ecrits += h_writes(num_soc, buffer, nb_octets_buffer);
	}
	// write() un délimiteur de fin ?
	
	// La fermeture de la connexion avec le client se fait en dehors de cette fonction 
	return nb_octets_ecrits;
}


