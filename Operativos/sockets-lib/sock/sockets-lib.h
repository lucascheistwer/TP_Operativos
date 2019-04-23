/*
 * sockets-lib.h
 *
 *  Created on: 16 abr. 2019
 *      Author: utnso
 */

#ifndef SOCKETS_LIB_H_
#define SOCKETS_LIB_H_

#include <unistd.h>
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
char package[PACKAGESIZE];
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
struct addrinfo hints;
struct addrinfo *serverInfo;

typedef struct{

	char* header;
	uint32_t message_long_header;
	char* query;
	uint32_t message_long_query;

	uint32_t total_size;			// NOTA: Es calculable. Aca lo tenemos por fines didacticos!
} t_Package_Request;
typedef struct{

	char* message;
	uint32_t message_long;
	uint32_t total_size;			// NOTA: Es calculable. Aca lo tenemos por fines didacticos!
} t_Package_Response;
int levantarCliente(char* puerto,char* ip);
int levantarServidor(char* puerto);
void enviarMensaje(int clienteSocket);
int aceptarCliente(int serverSocket);
void recibirMensaje(int socketServidor);
void cerrarConexiones(int socket_cliente,int socket_servidor);
char* serializarRequest(t_Package_Request *package);
char* serializarResponse(t_Package_Response *package);
void llenarPaqueteRequest(t_Package_Request *package,char msg[]);
void llenarPaqueteResponse(t_Package_Response *package);
char *append(const char *oldstring, const char c);
void dispose_package(char **package);
#endif /* SOCKETS_LIB_H_ */
