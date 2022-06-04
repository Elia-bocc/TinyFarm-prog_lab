# TinyFarm-prog_lab
Progetto LaboratorioII

	GESTIONE SEGNALI
		Gestisco i segnali attraverso un thtread gestore designato al quale passo
		il corpo da eseguire e una struct contenente il puntatore ad una variabile
		di tipo volatile sig_atomic_t utilizzata per gestire l'inserimento degli 
		elementi all'interno del buffer produttore/cosumatore, quando viene settata
		a false il produttore smette di inviare dati ai consumatori e svolge tutte
		le operazioni per terminare il programma correttamente.
		Per quanto riguarda le maschere il thread gestore riceve solamente SIGINT mentre
		tutti gi altri thread (compreso quello principale) ricevono solo SIGQUIT.
		Ho deciso di utilizzare un thread a parte per la gestione dei segnali per non
		disturbare gli altri threads che stanno svolgendo operazioni e per non utilizzare
		variabili globali.

	TRASMISSIONE PROD/CONS
		Per la trasmissione prod/cons ho utilizzato il classico metodo prod/cons, dove 
		il produttore era il thread principale, il quale lanciava nthread threads che 
		erano i consumatori; ai threads consumatori ho dovuto passare una struct contenente
		un buffer di puntatori a carattere di grandezza qlen, un mutex per evitare conflitti 
		sulla lettura da buffer, l'indice del buffer relativo ai consumatori e due semafori
		sem_free_slots e sem_data_items che indicano rispettivamente il numero di slot liberi
		per inserire dati, e il numero di elementi disponibili per la lettura dal buffer.


	TERMINAZIONE CONSUMATORI
		Per la terminazione dei consumatori scrivo semplicemente nel buffer una 
		stringa che non può essere il nome di un file, ovvero "t" che non ha l'estensione,
		il consumatore lo legge e capisce che deve terminare.
		