/*
 * Kernel.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "Kernel.h"


#define IP "127.0.0.1"
#define PUERTOMEM "6667"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
char package[PACKAGESIZE];
struct addrinfo hints;
struct addrinfo *serverInfo;

// Define cual va a ser el size maximo del paquete a enviar

int main() {
	int socket_memoria = levantarCliente(PUERTOMEM,IP);
	pthread_t hiloenvio;
	pthread_create(&hiloenvio, NULL, (void*) enviarMensaje, (int*) socket_memoria);
	pthread_join(hiloenvio,NULL);
	close(socket_memoria);


	return 0;
}
/*
void recibirMensajeDeMemoria(int serverSocket) {
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
		socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(serverSocket, (struct sockaddr *) &addr, &addrlen);
	char package[PACKAGESIZE];
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Cliente conectado. Esperando mensajes:\n");

	while (status != 0) {
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if (status != 0)
			printf("%s", package);
	}
}

void enviarMensajeAMemoria(int clienteSocket) {
	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while (enviar) {
		fgets(message, PACKAGESIZE, stdin);	// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message, "exit\n"))
			enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar)
			send(clienteSocket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
	}
}

int levantarCliente() {

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTOESCRIBE, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);


	while(connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) != 0){
		connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return serverSocket;
}

int levantarServidor() {

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTOESCUCHA, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(listenningSocket, BACKLOG);

	return listenningSocket;
}

void cerrarConexiones(int socket_cliente, int socket_servidor) {
	close(socket_cliente);
	close(socket_servidor);
}


typedef struct _t_Package {

	char* message;
	uint32_t message_long;
	uint32_t total_size;			// NOTA: Es calculable. Aca lo tenemos por fines didacticos!
} t_Package;

char* serializarOperandos(t_Package *package){

	char *serializedPackage = malloc(package->total_size);

	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(package->message_long);
	memcpy(serializedPackage + offset, &(package->message_long), size_to_send);
	offset += size_to_send;

	size_to_send =  package->message_long;
	memcpy(serializedPackage + offset, package->message, size_to_send);

	return serializedPackage;
}




 * 	Funciones auxiliares


void fill_package(t_Package *package){
	// Me guardo los datos del usuario y el mensaje que manda
 	// Me guardo lugar para el \0



	fgets(package->message, MAX_MESSAGE_SIZE, stdin);
	(package->message)[strlen(package->message)] = '\0';

	package->message_long = strlen(package->message) + 1;	// Me guardo lugar para el \0

	package->total_size = sizeof(package->message_long) + package->message_long;
	// Si, este ultimo valor es calculable. Pero a fines didacticos la calculo aca y la guardo a futuro, ya que no se modificara en otro lado.
}

void dispose_package(char **package){
	free(*package);
}*/
