/*
 * Copyright (C) 2002, Simon Nieuviarts
 *               2010, Gregory Mounie
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include <signal.h>
#include <glob.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define VARIANTE "Jokers  ́tendus (tilde, brace) (sec. 5.2) ; Terminaison asynchrone (sec. 5.4) ;"


int nbrPTF=0; //Nbre de processus en tache de fond, //
Liste listeps; //Liste utile pour stocker les processus en tache de fond //




//Fonction handler a l'interception d'un signal fils//
void handler(int Sig){
	int pid;
	
	//--------On reactualise la liste des processus en tache de fond -------------//	
	//On va parcourir la listeps avec liste fictive
	Liste fictif=calloc(1,sizeof(*fictif));;
	fictif->suiv=listeps;;
	Liste parcours=fictif;
	Liste erase;
	
	//On supprime de la liste les processus terminés //
	while(!est_vide(parcours->suiv)){
	pid=waitpid((parcours->suiv)->pid,NULL,WNOHANG);
	/* Pour chaque processus, on regarde s'il est terminé */
	if(pid>0) { 
	/* On annonce la terminaison du processus */
		printf("[%d]+   Done\n",parcours->suiv->rang);
		if(parcours->suiv->rang==nbrPTF) {
		nbrPTF--;
		}
		/* le processus est terminé, on le supprime de la liste */
		erase=parcours->suiv;
		parcours->suiv=parcours->suiv->suiv;
		freeTache(erase);
		free(erase);
	}
	else {
		parcours=parcours->suiv;
	}
	}	
	listeps=fictif->suiv;
	free(fictif);		
}




int main()
{	
	listeps=creer_liste(); //On initialise la liste des taches de fonds

	printf("%s\n", VARIANTE);
	while (1) {
		struct cmdline *l;
		int i, j;
		char *prompt = "ensishell>";

		l = readcmd(prompt);

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
/*		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }
			printf("\n");
		}*/
		
//////////////////////////////////////////////////*Debut du TP*//////////////////////////////////////////////////////////////:

		int pidfils;
		int status;
		int nbseq=0;
		for (i=0; l->seq[i]!=0; i++) {
		    nbseq++;
		}
		int pid[nbseq]; //tableau des pids des processus fils
		int nbtache=0; //nbre de processus qui ne sont pas en tache de fond
		//Variables utiles pour les tuyaux//
		int Tuyau[2]; 
		pipe(Tuyau);	

	//-------Traitement taches de fond------------------//		
	 /* => voir handler ( traitement terminaison asynchrone */
	
	
	//-------Traitement Terminaison asynchrone------------------//	
		
		struct sigaction traitant = {};
		traitant.sa_handler = handler;
		sigemptyset ( & traitant.sa_mask );
		traitant.sa_flags =0;
		if ( sigaction( SIGCHLD, &traitant, 0) == -1 )
		perror("sigaction");
		
		
	/*-------Traitement Jokers etendus------------------*/
	  /*   ===>Voir readcmd.c   */
	
	
	 /*-------- On lit la sequence de commande --------------*/
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			 //Traitement des instructions//
			 //Cas liste_ps//
			 if (strcmp(cmd[0],"liste_ps")==0) {
			      visualiser(listeps);
			 }
			 //Cas exit//
			 if (strcmp(cmd[0],"exit")==0) {
			      exit(0);
			 }


			 else {
			      /*Execution des commandes de l'entrée standard*/
			      pidfils=fork(); //cas d'erreur -1 a traiter?//
			      if (pidfils==0) {
				   
				   //On relie les commandes//
				   //Cas: Premiere sequence de commandes//
				   if(i==0) {

					// Redirection du fichier in sur l'entrée de la premiere commande
					if(l->in!=NULL) {          
					     int STDIN=open(l->in,O_RDONLY);
					     if(STDIN==-1) {
					     	printf("Erreur dans l'ouverture du fichier IN\n");
					     	exit(0); //On ouvre le descripteur de fichier mode lecture
					     }
					     
					     dup2(STDIN,0); // On le duplique dans le descripteur stdin//
					     if(close(STDIN)==-1){
						  printf("Erreur dans la fermeture du fichier\n");
					     }
					} 
					//Si nous n'avons pas d'autres commandes alors on redirige la sortie de la premiere commande sur le fichier out  
					if(l->seq[i+1]==0) {
					
					     if(l->out!=NULL) {
					     	int STDOUT=open(l->out, O_RDWR |O_CREAT | O_TRUNC,S_IRWXU | S_IRWXG | S_IRWXO); //ouverture en mode lecture/ecriture avec les droits necessaires//
					     	if (STDOUT==-1) {
					     		printf("Erreur dans l'ouverture du fichier OUT\n");
					     		exit(0);
						}
						
						dup2(STDOUT,1); // On le duplique dans le descripteur stdout//
						if(close(STDOUT)==-1){
						        printf("Erreur dans la fermeture du fichier\n");
						}
					     }
					}
					else { //Sinon on relie les sequences de commandes entre elles, en l'occurence ici, on dirige la sortie de la commande vers l'entrée du tuyau//			
					     dup2(Tuyau[1],1); // on relie la sortie du processus au pipe coté ecriture//
					     close(Tuyau[1]); //Cloture l'ecriture du pipe //
					     close(Tuyau[0]); //Cloture la lecture du pipe//
					}
				   }	
				   
				   //Cas: Deuxieme sequence de commandes//
				   else if( i==1) {
					//On relie la sortie du tuyau a l'entrée de la sequence de commandes
					dup2(Tuyau[0],0); // on relie l'entree du processus au pipe coté lecture//
					close(Tuyau[1]); //on cloture l'ecriture du pipe//
					close(Tuyau[0]); //on cloture la lecture du pipe //	
				   

					//On gere la redirection de sortie si elle existe//
					if(l->out!=NULL) {
					     int STDOUT;
					     if ((STDOUT=open(l->out, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO))==-1) {
					     	printf("Erreur dans l'ouverture du fichier OUT");
					        exit(0);
					     }
					     	
					    //On ouvre le descripteur de fichier mode ecriture, creation et concatenation//
					     dup2(STDOUT,1);
					     if(close(STDOUT)==-1){
						  printf("Erreur dans la fermeture du fichier\n");
					     }

				         }  
				   }
			 	   	      
			      //On execute la commande
			       execvp(cmd[0],cmd);
			       perror("exec");
			       exit(1);

			      }
			  else {		  
			      if (l->bg) {// Cas tache de fond//  
				   nbrPTF++;
				   listeps=ajout_queue(cmd,pidfils,nbrPTF,listeps);  //On ajoute le processus à la liste// 
			           printf("[%d] %d\n",nbrPTF, pidfils);	//On declare la tache de fond//	                     
			      }
			      else {
				   pid[nbtache]=pidfils; //on stocke les pids des processus fils qui ne sont pas des tâches de fonds dans le tableau	   
				   nbtache++;
				   if(i==0){
					waitpid(pidfils,&status,0); //Necessaire pour faire passer le test 6?!
				   }	
			      }	   
	   
			  }
			 }	      
		
		}
	close(Tuyau[0]);
	close(Tuyau[1]);

	for(i=0;i<nbtache;i++) {
	  waitpid(pid[i],&status,0); //On attend la fin des processus//
	}    				
			 
	}	


}
