# TinyFarm-prog_lab
Progetto LaboratorioII

	GESTIONE SEGNALI
		Gestisco i segnali attraverso un thread gestore designato al quale passo
		il corpo da eseguire e una struct contenente il puntatore ad una variabile
		di tipo volatile sig_atomic_t utilizzata per gestire l'inserimento degli 
		elementi all'interno del buffer produttore/cosumatore, quando viene settata
		a false il produttore smette di inviare dati ai consumatori e svolge tutte
		le operazioni per terminare il programma correttamente.
		Per quanto riguarda le maschere il thread gestore riceve solamente SIGINT, che setta
		a true la variabile di controllo, e SIGUSR2 che fa terminare il thread, mentre
		tutti gi altri thread (compreso quello principale) ricevono solo SIGQUIT.
		Ho deciso di utilizzare un thread a parte per la gestione dei segnali per non
		disturbare gli altri threads che stanno svolgendo operazioni e per non utilizzare
		variabili globali.

	TRASMISSIONE PROD/CONS
		Per la trasmissione prod/cons ho utilizzato il classico metodo prod/cons, dove 
		il produttore è il thread principale, il quale lancia nthread threads che 
		sono i consumatori; ai threads consumatori ho dovuto passare una struct contenente
		un buffer di puntatori a carattere di grandezza qlen, un mutex per evitare conflitti 
		sulla lettura da buffer, l'indice del buffer relativo ai consumatori e due semafori
		sem_free_slots e sem_data_items che indicano rispettivamente il numero di slot liberi
		per inserire dati, e il numero di elementi disponibili per la lettura dal buffer.


	TERMINAZIONE CONSUMATORI
		Per la terminazione dei consumatori scrivo semplicemente nel buffer la stringa
		"t", il consumatore la legge, esce dal ciclo while e termina con pthread_exit().

	FORMATO DATI CLIENT/SERVER
		il formato con cui vengono inviati i dati è così definito:
	
			lunghezza della somma | lunghezza del nome del file | somma | nome file
	 
		dove le lunghezze sono short, mentre la somma e il nome del file sono stringhe.
	
		Ho scelto questo formato cosicché al momento dela ricezione è già chiara la dimensione
		dei dati in arrivo

	SERVER
		L'eseguibile collector.py è un server in grado di gestire più richieste client
		simultaneamente; quindi si mette in ascolto di richieste e una volta ricevuta la affida ad
		un thread dedicato, in modo da non bloccare l'intero server su una singola richiesta
		

	TERMINAZIONE CONNESSIONE CLIENT/SERVER
		Per la terminazione della connessione, il client C, dopo che i threads consumatori hanno
		inviato tutti i dati e sono terminati, invia al server il valore 0 come dimensione della 
		somma (cosa impossibile se ci fosse stato un qualsiasi valore); quindi il server come 
		prima cosa controlla il primo dato inviato, se è 0 lancia il segnale SIGINT che viene
		gestito dal try/except nel thread principale sbloccando l'attesa dalla s.accept(), esce 
		dal ciclo while, fa la join dei threads ed infine termina la connessione; se il messaggio
		di terminazione non arriva allora procede alla ricezione della restante parte dei dati
		che poi stamperò.

	PER ALTRI DETTAGLI CI SONO I COMMENTI NEL CODICE
	