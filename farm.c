#include "xerrori.h"

#define nthread 4
#define qlen 8
#define delay 0


/*
Da fare:
	gestione parametri opzionali;
	decidere se fare le join sul server o sul masterworker
	connessione al server dei worker e inviare i dati
	tutta la parte successiva del produttore
	controllare alcuni dettagli segnati
*/




/*
	il thread principale lancia n threads e successivamente gli dà in pasto i nomi dei
	file ricevuti sulla linea di comando attraverso un buffer prod/cons; alla fine 
	manda un messaggio di terminazione al server e successivamente attende la terminazione
	dei threads.
	I threads lanciati dal principale prendono il nome del file dal buffer prod/cons
	e calcolano una somma attraverso una formula, che poi verrà mandata al server 
	insieme al nome del file per essere stampata; saranno necessari opportuni
	metodi di sincronizzazione

	La gestione dei segnali sarà l'ultima cosa che farò
*/


// definisco i parametri da passare ai threads worker
typedef struct {
  int *cindex;  // indice nel buffer
  char **buffer; 
	pthread_mutex_t *cmutex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;  
} dati;

// funzione eseguita dai thread worker
void *tbody(void *arg)
{  
  dati *a = (dati *)arg; 
  char *n_file;  // da guardare bene di non creare problemi
  while(true) {
    xsem_wait(a->sem_data_items,__LINE__,__FILE__);
		xpthread_mutex_lock(a->cmutex,__LINE__,__FILE__);
    n_file = a->buffer[*(a->cindex) % qlen];
    *(a->cindex) +=1;
		xpthread_mutex_unlock(a->cmutex,__LINE__,__FILE__);
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
		//meccanismo di terminazione dei threads worker
		if (n_file == NULL) break;
		//apro il file
		FILE *f = fopen(n_file, "rb");
		termina("Errore lettura da file");
		// stabilisco il numero di long
		int e = fseek(f, 0, SEEK_END);
		if (e!=0) termina("errore fseek");
		long t = ftell(f);
		if (t<0) termina("errore ftell");
		int n_long = t/sizeof(long);
		rewind(f);
		//calcolo somma da file
		long somma = 0;
		size_t e2;
		long num;
		for (int i=0; i<n_long; i++) {
			e2 = fread(&num, sizeof(long), 1, f);  //non so se va bene
			if (e2 == 0) break;
			somma += i*num;
		}
		fclose(f);
		
		//connessione al server e invio somma e n_file



		
  }
  pthread_exit(NULL); 
}   

int main(int argc, char *argv[]) {
  // controlla numero argomenti
  if(argc<2) {
      printf("Uso: %s file [file ...] \n",argv[0]);
			termina("Errore dati input");
  }
	
	//gestione parametri opzionali




	

	
  // threads related
  char *buffer[qlen];
  int pindex=0,cindex=0;
	pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t t[nthread];
  dati a[nthread];
  sem_t sem_free_slots, sem_data_items;
  xsem_init(&sem_free_slots,0,qlen,__LINE__,__FILE__);
  xsem_init(&sem_data_items,0,0,__LINE__,__FILE__);
  for(int i=0;i<nthread;i++) {
    // faccio partire il thread i
    a[i].buffer = buffer;
		a[i].cindex = &cindex;
		a[i].cmutex = &cmutex;
    a[i].sem_data_items = &sem_data_items;
    a[i].sem_free_slots = &sem_free_slots;
    xpthread_create(&t[i],NULL,tbody,a+i,__LINE__,__FILE__); //da controllare
  }
	
	//produttore




	


	//terminazione threads



	//join threads ?????



	
	//terminazione server

	


	
  return 0;
}