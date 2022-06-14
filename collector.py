#! /usr/bin/env python3
"""
import sys, struct, socket, threading, signal, os

# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
controllo = False


def term(host, port):
	#setto a True la variabile di controllo
	global controllo
	controllo = True
	# inizializzazione socket client 
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		# la prossima chiamata è blocking
		s.connect((host, port))
		print("Connesso a", s.getpeername())
		print("finito")


def main(host=HOST,port=PORT):
	# creiamo il server socket
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.bind((host,port))
		s.listen()
		n = 0
		t = []
		while True:
			# mi metto in attesa di una connessione
			print(f"In attesa di un client")
			conn, addr = s.accept()
			if controllo:
				break
			# lavoro con la connessione appena ricevuta
			t.append(threading.Thread(target=gestisci_connessione, args=(conn,addr,n+1,t,s,host,port)))
			t[n].start()
			n += 1
		s.shutdown(socket.SHUT_RDWR)


# gestisci una singola connessione
# con un client
# prende come parametri la socket conn, l indirizzo addr, il numero del thread n
# e la socket del server s
def gestisci_connessione(conn,addr, n, t, s,host,port): 
  # in questo caso potrei usare direttamente conn
  # e l'uso di with serve solo a garantire che
  # conn venga chiusa all'uscita del blocco
  # ma in generale with esegue le necessarie
  # inzializzazione e il clean-up finale
	with conn:  
		print(f"Contattato da {addr}")
		# ---- ricevo uno short (dimensione long), che potrebbe essere
		# il messaggio di terminazione
		data = recv_all(conn,2)
		assert len(data)==2
		l_long = struct.unpack("!h",data)[0]
		# controllo se è stato mandato il messaggio di terminazione
		if l_long == 0:
			# aspetto la terminazione degli altri threads
			assert(n>=1)
			for i in range(n-1):
				t[i].join()
			term(host,port)
			#s.close()
		else:
			
			# se il server non termina ricevo la dimensione del nome del
			#file, il long e il nome del file
			data = recv_all(conn,2)
			assert len(data)==2
			l_file = struct.unpack("!h",data)[0]
			data = recv_all(conn,l_long)
			assert len(data) == l_long
			somma = data.decode()
			data = recv_all(conn,l_file)
			assert len(data) == l_file
			n_file = data.decode()
				
    	# ---- stampo su stdout
			print(f"Somma: {somma} File: {n_file}")

"""
import sys, struct, socket, threading, signal, os


# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)


def main(host=HOST,port=PORT):
  # creiamo il server socket
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		try:
			s.bind((host, port))
			s.listen()
			n = -1
			t = []
			while True:
			# mi metto in attesa di una connessione
				print(f"In attesa di un client")
				conn, addr = s.accept()
				n += 1
				# lavoro con la connessione appena ricevuta
				# passo come argomenti la connessione l indirizzo
				# l'identificativo del thread viene memorizzato in una lista per poi poter fare la join
				t.append(threading.Thread(target=gestisci_connessione, args=(conn,addr)))
				t[n].start()
		except  KeyboardInterrupt:
			pass
		# aspetto la terminazione dei threads e termino la connessione
		for i in range(n+1):
			t[i].join()
		s.shutdown(socket.SHUT_RDWR)



# gestisci una singola connessione
# con un client
# prende come parametri la socket conn, l indirizzo addr,
# il pid del thread principale
def gestisci_connessione(conn,addr): 
  # in questo caso potrei usare direttamente conn
  # e l'uso di with serve solo a garantire che
  # conn venga chiusa all'uscita del blocco
  # ma in generale with esegue le necessarie
  # inzializzazione e il clean-up finale
	with conn: 
		print(f"Contattato da {addr}")
		# ricevo uno short (dimensione del long)
		data = recv_all(conn,2)
		assert len(data)==2
		l_long = struct.unpack("!h",data)[0]
		# controllo se è stato mandato il messaggio di terminazione
		if l_long == 0:
			# invio SIGINT per terminare il server
			os.kill(os.getpid(), signal.SIGINT)
		else:
			# se il messaggio di term. non arriva ricevo la dimensione del nome 
			#del file, il long e il nome del file
			data = recv_all(conn,2)
			assert len(data)==2
			l_file = struct.unpack("!h",data)[0]
			data = recv_all(conn,l_long)
			assert len(data) == l_long
			somma = data.decode()
			data = recv_all(conn,l_file)
			assert len(data) == l_file
			n_file = data.decode()
				
    	# stampo su stdout
			print(f"{somma} {n_file}")
 

# riceve esattamente n byte e li restituisce in un array di byte
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

main()