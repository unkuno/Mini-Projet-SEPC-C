#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/*----------------- Sujet 5 : Lecteur-rédacteur sémaphores et Producteur-consommateur moniteurs*/

int nblect=0;
sem_t mutex;
sem_t BD;
sem_t SAS;

pthread_mutex_t mutex2;
pthread_cond_t fprod;
pthread_cond_t fconso;
int Nmess=2;
int Pconso=0;
int Pprod=0;
int tampon[2];





/*---------------- TP Lecteur-redacteur sémaphore priorité rédacteur --------------------*/ 

void debut_lire (){
	printf("Demande de lecture\n");
	sem_wait(&SAS); // Sas permettant de bloquer l'arrivée de lecteurs après l'arrivée d'un écrivain
	sem_wait(&mutex); /*Exclusion mutuelle*/
	nblect++;
	if(nblect==1){
		sem_wait(&BD);
		printf("Blocage de la base de donnée pour les redacteurs\n");
	}
	sem_post(&mutex);
	sem_post(&SAS); // sortie du sas afin de pouvoir laisser passer plusieurs lecteurs
	printf("Debut de la lecture (je suis le %d ième à lire)\n",nblect);

}


void fin_lire (){
	sem_wait(&mutex);
	printf("Fin de lecture (il reste %d lecteurs en train de lire)\n",nblect-1);
	nblect--;
	if(nblect==0){
		printf("Deblocage de la base de donnée pour les rédacteurs\n");
		sem_post(&BD);
	}
	sem_post(&mutex);    
}     

void debut_ecrire () {
	printf("Demande d'ecriture\n");
	sem_wait(&SAS); // si un écrivain arrive on bloque l'arrivée des lecteurs afin d'éviter la famine des écrivains
	sem_wait(&BD);
	sem_post(&SAS); 
	printf("Debut de l'ecriture\n");
}

void fin_ecrire (){
	printf("Fin de l'ecriture\n");
	sem_post(&BD);
}     

/*------------------ TP Producteur-Consommateur Moniteur----------------------*/ 

void deposer(int message){
	pthread_mutex_lock(&mutex2);
	while(Nmess==0){
		pthread_cond_wait(&fprod,&mutex2);
	}
	printf("		Ecriture du message :  %d dans le tampon %d\n", message,Pprod);
	tampon[Pprod]=message;
	Nmess--;
	Pprod=(Pprod+1)%2;     
	pthread_cond_signal(&fconso);
	pthread_mutex_unlock(&mutex2);
}

void retirer() {     
     pthread_mutex_lock(&mutex2);
     while(Nmess==2) {
	  pthread_cond_wait(&fconso,&mutex2);
     }
     Nmess++;
     int message=tampon[Pconso];
	 printf("		Message : %d lue dans le tampon %d\n",message,Pconso);  
     Pconso=(Pconso+1)%2;
     pthread_cond_signal(&fprod);

     pthread_mutex_unlock(&mutex2);
}     
     



void *thread1(void *a)
{
     int i= (int) a;
     for(i=0;i <10; i++)
     {
	  deposer(i);
	  usleep(100 * drand48());
	  debut_lire();
	  fin_lire();
     }	  
}

void *thread2(void *a)
{
     int i= (int) a;
     for(i=0;i <10; i++)
     {
	  retirer();
	  usleep(100 * drand48());
	  debut_ecrire();
	  fin_ecrire();
     }	  
}

void *thread3(void *a)
{
     int i= (int) a;
     for(i=0;i <10; i++)
     {
	  deposer(i);
	  usleep(100 * drand48());
	  debut_ecrire();
	  fin_ecrire();
     }	  
}


void *thread4(void *a)
{
     int i= (int) a;
     for(i=0;i <10; i++)
     {
	  retirer();
	  usleep(100 * drand48());
	  debut_lire();
	  fin_lire();
     }	  
}


main() {
void * status;
pthread_t thread1_pid,thread2_pid,thread3_pid,thread4_pid,thread5_pid,thread6_pid,thread7_pid,thread8_pid;


/*initialisation des semaphores*/
sem_init(&mutex,0,1);
sem_init(&BD,0,1);
sem_init(&SAS,0,1);

pthread_create(&thread1_pid,NULL,thread1,NULL);
pthread_create(&thread2_pid,NULL,thread2,NULL);
pthread_create(&thread3_pid,NULL,thread3,NULL);
pthread_create(&thread4_pid,NULL,thread4,NULL);
pthread_create(&thread5_pid,NULL,thread1,NULL);
pthread_create(&thread6_pid,NULL,thread2,NULL);
pthread_create(&thread7_pid,NULL,thread3,NULL);
pthread_create(&thread8_pid,NULL,thread4,NULL);
     
pthread_join(thread1_pid,&status);
pthread_join(thread2_pid,&status);
pthread_join(thread3_pid,&status);
pthread_join(thread4_pid,&status);
pthread_join(thread5_pid,&status);
pthread_join(thread6_pid,&status);
pthread_join(thread7_pid,&status);
pthread_join(thread8_pid,&status);
}     