/*
 * sockets-lib.c
 *
 *  Created on: 16 abr. 2019
 *      Author: utnso
 */

#include "sockets-lib.h"

#include <stdio.h>
int levantarCliente(char* puerto,char* ip) {

	memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype =SOCK_STREAM; // Indica que usaremos el protocolo TCP

		getaddrinfo(ip, puerto, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

		int serverSocket;
		serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol);


		while(connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) != 0){
			connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
		}
		freeaddrinfo(serverInfo);	// No lo necesitamos mas
		return serverSocket;
}

int levantarServidor(char* puerto) {

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, puerto, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(listenningSocket, BACKLOG);

	return listenningSocket;
}

void enviarMensaje(int clienteSocket) {
	int enviar = 1;
	char message[PACKAGESIZE];
	t_Package_Request package;
	char *serializedPackage;
	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while (enviar) {
		fgets(message, PACKAGESIZE, stdin);	// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message, "exit\n"))
			enviar = 0;			// Chequeo que el usuario no quiera salir
		llenarPaqueteRequest(&package,message);
		if (enviar)
			serializedPackage =serializarRequest(&package);
			send(clienteSocket, serializedPackage, package.total_size, 0); // Solo envio si el usuario no quiere salir.
			dispose_package(&serializedPackage);
			recv(clienteSocket, (void*) message, PACKAGESIZE, 0);
		printf("%s\n", package.header);
	}
}

int aceptarCliente(int serverSocket) {
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(serverSocket, (struct sockaddr *) &addr, &addrlen);
		printf("Cliente conectado. Esperando mensajes:\n");
		recibirMensaje(socketCliente);
		return socketCliente;
}
void recibirMensaje(int socketServidor){

	int status = 1;		// Estructura que manjea el status de los recieve.

	t_Package_Request package;
	while (status != 0) {
		int status;
			int buffer_size;
			char *buffer = malloc(buffer_size = sizeof(uint32_t));

			uint32_t header_long;
			status = recv(socketServidor, buffer, sizeof(package.message_long_header), 0);
			memcpy(&(header_long), buffer, buffer_size);
			if (!status) status=0;

			status = recv(socketServidor, package.header, header_long, 0);
			if (!status) status=0;

			uint32_t message_long_query;
			status = recv(socketServidor, buffer, sizeof(package.message_long_query), 0);
			memcpy(&(message_long_query), buffer, buffer_size);
			if (!status) status=0;

			status = recv(socketServidor, package.query, message_long_query, 0);
			if (!status) status=0;


			free(buffer);
		if (status != 0)
			printf("%s", package);
		send(socketServidor, "Recibi tu msg",14, 0);
	}

}
char* serializarRequest(t_Package_Request *package){

	char *serializedPackage = malloc(package->total_size);

	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(package->message_long_header);
	memcpy(serializedPackage + offset, &(package->message_long_header), size_to_send);
	offset += size_to_send;

	size_to_send =  package->message_long_header;
	memcpy(serializedPackage + offset, package->header, size_to_send);

	size_to_send =  sizeof(package->message_long_query);
	memcpy(serializedPackage + offset, &(package->message_long_query), size_to_send);
	offset += size_to_send;

	size_to_send =  package->message_long_query;
	memcpy(serializedPackage + offset, package->query, size_to_send);

	return serializedPackage;
}



void llenarPaqueteRequest(t_Package_Request *package,char msg[]){
	// Me guardo los datos del usuario y el mensaje que manda
 	// Me guardo lugar para el \0


	char header[20]="";int i=0;
	char car[2];
	car[0]=msg[i];
	while(car[0]!=' '){
	strcat(header,car);
		car[0]=msg[i++];
	}
	i++;
	char resto[100]="";
	while(car[0]!='\0'){
		strcat(resto,car);
		car[0]=msg[i++];
	}
	package->message_long_header= strlen(header) + 1;
	package->message_long_query= strlen(resto) + 1;
	strcpy(package -> header,header);
	strcpy(package -> query,resto);

	(package->header)[strlen(package->header)] = '\0';

	(package->query)[strlen(package->query)] = '\0';

	package->total_size = sizeof(package->message_long_header) + package->message_long_header+ sizeof(package->message_long_query) + package->message_long_query;

	// Si, este ultimo valor es calculable. Pero a fines didacticos la calculo aca y la guardo a futuro, ya que no se modificara en otro lado.
}


void dispose_package(char **package){
	free(*package);
}
int split (const char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}
