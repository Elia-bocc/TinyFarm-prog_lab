#include "xerrori.h"
#define HOST "127.0.0.1"
#define PORT 65432 

/*
Da fare:
	-connessione al server dei worker e inviare i dati
	-controllare alcuni dettagli segnati
	-testare separatamente eseguibile c e server python
*/

/*
Potrei gestire la trasmissione server/client inviando un primo messaggio 
che stabilisce la grandezza del nome del file, così che se la dimesione è zero 
posso terminare il server (messaggio di terminazione), e così che so già quanti bytes dovrò aspettare di leggere.
Quindi poi come secondo messaggio la somma e come terzo il nome del file.
Perciò sul server dovrò aggiungere un parametro per la dimensione del nome del file
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


//gestore per il segnale SIGNIT

typedef struct {
  volatile sig_atomic_t *c;
} dgest;

void *tgestore(void *v) {
	dgest *a = (dgest *)v;
  sigset_t mask;
  sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
  int s;
  while(true) {
    int e = sigwait(&mask,&s);
		sleep(6);
    if(e!=0) perror("Errore sigwait");
		if (s == SIGINT) {
			a->c = false;
		}
		else xtermina("errore ricezione segnale", __LINE__,__FILE__);
    
  }
  return NULL;
}

// mi serve una funzione di scrittura per trasferimento dati al server

ssize_t writen(int fd, void *ptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) < 0) {
			if (nleft == n) return -1;
			else break;
		}
		else if (nwritten == 0) break;
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n - nleft);
}


// definisco i parametri da passare ai threads worker
typedef struct {
  int *cindex;  // indice nel buffer
  char **buffer; 
	int qlen;
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
    n_file = a->buffer[*(a->cindex) % a->qlen];
    *(a->cindex) +=1;
		xpthread_mutex_unlock(a->cmutex,__LINE__,__FILE__);
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
		//meccanismo di terminazione dei threads worker
		if (strcmp(n_file, "t")==0) break;
		//apro il file
		FILE *f = fopen(n_file, "rb");
		if(f==NULL) termina("Errore lettura da file");
		// stabilisco il numero di long
/*
		long num;
		long somma = 0;
		size_t e2;
		int i = 0;
		while(true) {
			e2 = fread(&num, sizeof(long), 1, f);
			if (e2==0) break;
			somma += i*num;
			i += 1;
		}
		*/
		int e = fseek(f, 0, SEEK_END);
		if (e!=0) termina("errore fseek");
		long t = ftell(f);
		if (t<0) termina("errore ftell");
		int n_long = t/sizeof(long);
		rewind(f);
		//calcolo somma da file
		long somma = 0;
		size_t e2;
		long *num = malloc(n_long*sizeof(long *));
		e2 = fread(num, sizeof(long), n_long, f);  //non so se va bene
		if (e2 != n_long) termina("errore lettura");
		for (int i=0; i<n_long; i++) {
			somma += i*num[i];
		}
		free(num);
		fclose(f);
		//fprintf(stdout, "\nsomma: %ld, n_file: %s", somma, n_file);
		//connessione al server e invio somma e n_file

		int fd_socket = 0;
		int tmp;
		struct sockaddr_in serv_addr;
		if ((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			xtermina("Socket creation error", __LINE__,__FILE__);
		}
		//assegna indirizzo
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT);
		serv_addr.sin_addr.s_addr = inet_addr(HOST);
		//apro connessione
		if (connect(fd_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
			xtermina("errore connessione", __LINE__, __FILE__);

		//invio i dati
		//trasformo la somma in una stringa, calcolo la sua dimensione e la invio
		char sommac[256];
		sprintf(sommac, "%ld", somma);
		int l_sommac = strlen(sommac); //+1 per lo /0
		tmp = htonl(l_sommac);
		e2 = writen(fd_socket, &tmp, sizeof(tmp));
		if (e2 != sizeof(int))
			xtermina("errore write l_sommac", __LINE__, __FILE__);
		//calcolo la dimensione del nome del file e la invio
		int l_file = strlen(n_file);
		tmp = htonl(l_file);
		e2 = writen(fd_socket, &tmp, sizeof(tmp));
		if (e2 != sizeof(int)) 
			xtermina("errore write l_file", __LINE__, __FILE__);
		e2 = writen(fd_socket, sommac, sizeof(char)*l_sommac);
		if (e2 != l_sommac) 
			xtermina("errore write l_sommac", __LINE__, __FILE__);
		e2 = writen(fd_socket, n_file, sizeof(char)*l_file);
		if (e2 != l_file)
			xtermina("errore write n_file", __LINE__, __FILE__);
		
		xclose(fd_socket, __LINE__, __FILE__);

	}
  pthread_exit(NULL); 
}   

int main(int argc, char *argv[]) {
  // controlla numero argomenti
  if(argc<2) {
      printf("Uso: %s file [file ...] \n",argv[0]);
			termina("Errore dati input");
  }

	// definisce la maschera di tutti i threads compreso quello principale
	
	sigset_t mask;
  sigfillset(&mask);  // insieme di tutti i segnali
  sigdelset(&mask,SIGQUIT); // elimino sigquit
  pthread_sigmask(SIG_BLOCK,&mask,NULL); // blocco tutto tranne sigquit
	
	
	//gestione parametri opzionali

	int nthread = 4, qlen = 8, delay = 0, opt;
  while ((opt = getopt(argc, argv, "n:q:t:")) != -1) {
    switch (opt) {
      case 'n':
        nthread = atoi(optarg);
        break;
			case 'q':
        qlen = atoi(optarg);
        break;
      case 't':
        delay = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s [-n nthread] [-q qlen] [-t delay]\n",
        argv[0]);
        exit(EXIT_FAILURE);
      }
  }
	if (optind >= argc) { //anche se non ce n'è bisogno
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }

  // threads related
  char *buffer[qlen];
  int pindex=0,cindex=0;
	pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t t[nthread+1];
  dati a[nthread];
  sem_t sem_free_slots, sem_data_items;
  xsem_init(&sem_free_slots,0,qlen,__LINE__,__FILE__);
  xsem_init(&sem_data_items,0,0,__LINE__,__FILE__);
  for(int i=0;i<nthread;i++) {
    // faccio partire il thread i
    a[i].buffer = buffer;
		a[i].qlen = qlen;
		a[i].cindex = &cindex;
		a[i].cmutex = &cmutex;
    a[i].sem_data_items = &sem_data_items;
    a[i].sem_free_slots = &sem_free_slots;
    xpthread_create(&t[i],NULL,tbody,a+i,__LINE__,__FILE__); //da controllare potrebe essere il problema della memoria
  }

	//lancio il thread gestore
	
	volatile sig_atomic_t c = true;
	dgest arg;
	arg.c = &c;
	xpthread_create(&t[nthread],NULL,tgestore, &arg,__LINE__, __FILE__);

	
	//produttore
		for (int i=optind; i<argc; i++) {
		if (c == false) break;
		sleep(delay);
  	xsem_wait(&sem_free_slots,__LINE__,__FILE__);
		buffer[pindex++ % qlen] = argv[i];
    xsem_post(&sem_data_items,__LINE__,__FILE__);
	}
//puts("prima term threads");
	//terminazione threads

	for (int i=0; i<nthread; i++) {
		xsem_wait(&sem_free_slots,__LINE__,__FILE__);
		buffer[pindex++ % qlen] = "t";
    xsem_post(&sem_data_items,__LINE__,__FILE__);
	}
	
	//join threads e distruzione mutex

	for(int i=0;i<nthread;i++)
    xpthread_join(t[i],NULL,__LINE__,__FILE__);
	xpthread_mutex_destroy(&cmutex,__LINE__,__FILE__);

	//terminazione server
	
	//creo la socket
	int fd_socket = 0;
	int tmp;
	size_t e;
	struct sockaddr_in serv_addr;
	if ((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xtermina("Socket creation error", __LINE__,__FILE__);
	}
	//assegna indirizzo
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = inet_addr(HOST);
	//apro connessione
	if (connect(fd_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		xtermina("errore connessione", __LINE__, __FILE__);

	//invio i dati
	int dato = 0;
	tmp = htonl(dato);
	e = writen(fd_socket, &tmp, sizeof(tmp));
	if (e != sizeof(int))
		xtermina("errore write term", __LINE__, __FILE__);

	xclose(fd_socket, __LINE__, __FILE__);
	
  return 0;
}