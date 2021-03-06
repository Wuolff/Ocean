#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAXNAME 10
#define MAXTEXT 100

/**
 * fonction print_msg 
 * ------------------
 * 
 */
void print_msg(char *talker, char * chat) 
{
	fputs(talker, stdout);
	fputs(": ", stdout);
	fputs(chat, stdout);
	fflush(stdout);
}

/**
 * fonction read_header
 * --------------------
 * 
 * retourne le nom du client qui parle
 * 
 * inscrit à l'emplacement "&loglen" le premier octet de la socket puis à "username" un nombre 
 * "loglen" d'octets de la socket
 * 
 */
void read_header(int sock, char * username) 
{
	int loglen ;
	read(sock, &loglen, 1);
	read(sock, username, loglen);
}

int main(int argc, char * argv[])
{

	int pidFils;
	int port = 6543;
	char nom[30];
	char commandeWrite[80];
	socklen_t len_cadresse;//sizeof(struct sockaddr_in);
	
	/* déclaration socket et contexte d'adressage serveur */
	int ssocket;
	struct sockaddr_in adr;
	
	/* déclaration socket et contexte d'adressage client */
	int csocket;
	struct sockaddr_in cadresse;  
	
	
	/* s'il manque un argument */
	if (argc!=2)
	{
		fprintf(stderr,"Usage : %s port-number", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	/* si la socket ne se crée pas */
	if ((ssocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket rendez-vous");
		exit(1);
	}

	/* on récupère le nom de la mchine sur laquelle le serveur est lancé */
	if (gethostname(nom, 30)==-1)
	{
		perror("Qui suis-je ?");
		exit(1);
	}

	/* affichage d'un identifiant et du nom de la machine sur laquelle le serveur est lancé */
	printf("User: %s - %d; Machine: %s\n", getlogin(), geteuid(), nom);

	/* Paramétrage du contexte d'adressage serveur */
	adr.sin_family=AF_INET;
	adr.sin_port=htons(port);
	adr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* si la socket ne se bind pas avec son contexte d'adressage */
	if (bind(ssocket, (struct sockaddr *) &adr, sizeof(adr))==-1)
	{
		perror("bind");
		exit(1);
	}

	/* si la socket ne rentre pas en mode listen */
	if (listen(ssocket,1)==-1)
	{
		perror("listen");
		exit(1);
	}

	/* Création de la socket client comme acceptation de la socket serveur */
	csocket = accept(ssocket, (struct sockaddr *)&cadresse, &len_cadresse);
	
	/**
	 * -------------------------------------
	 * La connection est maintenant établie.
	 * -------------------------------------
	 */
	
	/* On peut fermer la socket serveur puisqu'elle n'est plus utilisée ... */
	close(ssocket);

	char c;
	char *talker = (char*)malloc(MAXNAME);
	char *chat =  (char*)malloc(MAXTEXT);
	char *begchat = chat;
	
	switch(pidFils=fork()) 
	{
		case -1:
		perror("fork");
		exit(1);
		
		case 0:
		/* Qui s'est connecté ? */
		read_header(csocket, talker);
		printf("%s is connected\n", talker);
		
		do
		{
			c = EOF;
			read(csocket, &c, 1);
			*chat = c;
			chat++;
			if (c == '\n' || c == EOF)
			{
				*chat = '\0';
				chat = begchat;
				print_msg(talker, chat);
			}
		}while (c!=EOF);
		
		fprintf(stderr,"Cote serveur: fin fils\n");
		
		break;
		default:
		do
		{
			c=getchar();
			write(csocket, &c, 1);
		}
		while (c!=EOF);

		kill(pidFils, SIGTERM);
		fprintf(stderr,"Cote serveur: fin pere\n");
	}
	return 0;
}




