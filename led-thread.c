/* Einfacher Mirrorserver in C mit Threads */
/* Kompilieren mit: 
 * gcc led-thread.c -o led-thread -Wall -lpthread -lwiringPi
 * (Linken mit pthread Library)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>

#define BUFSIZE 1000
#define PORT 4000
#define pin1 1
#define pin2 4
#define pin3 5
#define pin4 6

// Warteschlange fuer ankommende Verbindung
#define QUEUE 3

int rest3;

// *** LED-Funktion (Prototyp für pthread_create) ***
// arg: Pointer auf rec_socket
void *led(void* arg)
{
	wiringPiSetup();
	pinMode( pin1 , OUTPUT);
	pinMode( pin2 , OUTPUT);
	pinMode( pin3 , OUTPUT);
	pinMode( pin4 , OUTPUT);
	int clientfd = *(int *)arg; // typecast
	char inbuffer[BUFSIZE];
	char outbuffer[BUFSIZE];
	
while (1){
	
	int rest,rest1,rest2;
	int state,state1,state2,state3;
	
	// lesen der Zeichen aus dem Clientsocket --> inbuffer
	// count: Anzahl der gelesenen Bytes
	int count = read(clientfd, inbuffer, sizeof(inbuffer));
	int length = atoi(inbuffer);  // wandelt string in int um
	
	// Eingabe in binär umwandeln	
	rest = length / 2;
	state = length % 2;
	rest1 = rest / 2;
	state1 = rest % 2;
	rest2 = rest1 / 2;
	state2 = rest1 % 2;
	rest3 = rest2 / 2;
	state3 = rest2 % 2;
	
	// Daten 		
	if(length <= 15) {
		digitalWrite( pin1 , state );
		digitalWrite( pin2 , state1 );
		digitalWrite( pin3 , state2 );
		digitalWrite( pin4 , state3 );

	} else {
		digitalWrite( pin1 , 0 );
		digitalWrite( pin2 , 0 );
		digitalWrite( pin3 , 0 );
		digitalWrite( pin4 , 0 );
	}
	
	// Sendebuffer in Clientsocket schreiben
	write(clientfd, outbuffer, count);	
	
	// Verbindung zum Client getrennt
	if ( inbuffer[0] == 'q'){
		close(clientfd);
		return NULL;
		}
	}	
}

// *** Main ***
int main()
{
	
	int server_socket, rec_socket;
	unsigned int len;
	struct sockaddr_in serverinfo, clientinfo;

	// Serversocket konfigurieren
	server_socket = socket(AF_INET, SOCK_STREAM,0); // TCP
	serverinfo.sin_family = AF_INET; // IPv4
	// hoert auf allen Interaces: 0.0.0.0 bzw. :::
	serverinfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverinfo.sin_port = htons(PORT);
	
	// Verbinde Socket mit der IP-Adresse und Port
	if(bind(server_socket,(struct sockaddr *)&serverinfo,sizeof(serverinfo)) != 0) {
			printf("Fehler Socket\n");
			return 1; // Rueckgabe Fehlercode
	}
	
	listen(server_socket, QUEUE); // Server wartet auf connect vom Client
	
	// Endlosschleife Server zur Abarbeitung der Client-Anfragen
	while(1){	
		printf("Server wartet...\n");
		// Verbindung vom Client eingetroffen
		rec_socket = accept(server_socket,(struct sockaddr *)&clientinfo, &len);
		printf("Verbindung vom %s:%d\n", inet_ntoa(clientinfo.sin_addr), ntohs(clientinfo.sin_port));
		
		printf("Willkommen!\n");
		printf("Menue:\n");
		printf("mit Eingabe der Zahlen zw. 0 und 15 koennen Sie die LEDs steuern\n");
		printf("mit eingeben von ' q ' beenden Sie das Programm\n");
		
		pthread_t child; // Thread Struktur
		// Thread mit Funktion mirror(rec_socket) erzeugen
		if(pthread_create(&child, NULL, led, &rec_socket) != 0) {
			perror("child error"); // Fehlerfall: Abbruch
			return 1;
		}
		else { // Kind erzeugt:
			printf("Abgekoppelt!\n");
			pthread_detach(child); // abgekoppelt vom Hauptprozess
		}  
	}
	return 0;
}
