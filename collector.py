# problemi/considerazioni:
# per far si che la stampa sia coerente
# non dovrebbe essere necessario usare mutex perchè in python
# i threads sono eseguiti separatamente

# cose da fare:
# 1 il blocco try except non va bene, il testo chiede che il server
# termini attraverso un messaggio del thread worker
# !!!! Provo a gestire la terminazione del server dalla funzione gestisci_connessione
# passando come argomenti conn,addr e in più una lista/insieme di identificativi
# dei thread mandati fino a quel momento così che quando arriva il messaggio di 
# terminazione il thread corrente si mette a fare la join di tutti i threads
# e poi termina il serever

# 2 scrivere la funzione gestisci_connessione

# 3 vedere se c'è un modo per non atendere tutti e 263 i bytes alla recv_all
# nella funzione gestisci connessione

#! /usr/bin/env python3
# server che stampa su stdout la somma dei long del file e il nome del file
# gestisce più clienti contemporaneamente usando i thread
# Parte premendo il tasto Run 
# può essere usato con il client pclient.py 
import sys, struct, socket, threading


# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

def main(host=HOST,port=PORT):
  # creiamo il server socket
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s: 
    s.bind((host, port))
    s.listen()
		n = 0
    while True:
      # mi metto in attesa di una connessione
      conn, addr = s.accept()
      # lavoro con la connessione appena ricevuta
      t+n = threading.Thread(target=gestisci_connessione, args=(conn,addr,n,s))
      t+n.start()
			n += 1


# gestisci una singola connessione
# con un client
# prende come parametri la socket conn, l indirizzo addr, il numero del thread n
# e la socket del server s
def gestisci_connessione(conn,addr, n, s): 
  # in questo caso potrei usare direttamente conn
  # e l'uso di with serve solo a garantire che
  # conn venga chiusa all'uscita del blocco
  # ma in generale with esegue le necessarie
  # inzializzazione e il clean-up finale
  with conn:  
    print(f"Contattato da {addr}")
    # ---- attendo un long del C da 8 bytes == 64 bit e una stringa che assumiamo
		# sia <=255 caratteri quindi <=255 bytes in utf8
    data = recv_all(conn,263)
    assert len(data)==263
    somma  = struct.unpack("!l",data[:8])[0]
    n_file = struct.unpack("!c",data[8:])[0]
		
		# controllo se è stato mandato il messaggio di terminazione
		if n_file == "terminazione":
			# aspetto la terminazione degli altri threads
			assert(n>=1)
			for i in range(n):
				t+i.join()
			s.shutdown(socket.SHUT_RDWR)
		
		else
    	# ---- stampo su stdout
			print(f"{somma} {n_file}")
 


# riceve esattamente n byte e li restituisce in un array di byte
# il tipo restituto è "bytes": una sequenza immutabile di valori 0-255
# analoga alla readn che abbiamo visto nel C
def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks