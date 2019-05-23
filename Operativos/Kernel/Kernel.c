/*
 * Kernel.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "Kernel.h"

#define IP "127.0.0.1"
#define PUERTOMEM "7878"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
char package[PACKAGESIZE];
struct addrinfo hints;
struct addrinfo *serverInfo;

// Define cual va a ser el size maximo del paquete a enviar

int main() {
	int socket_memoria = levantarCliente(PUERTOMEM, IP);
	char consulta[256] = "";
	while (1) {

		fgets(consulta, 256, stdin);
		int consultaOk = despacharQuery(consulta, socket_memoria);
		if (!consultaOk) {
			printf("no entendi tu header \n");
		}
		consultaOk = 0;
	}

	//close(socket_memoria);
}

type validarSegunHeader(char* header) {
	if (!strcmp(header, "SELECT")) {
		return SELECT;
	}
	if (!strcmp(header, "INSERT")) {
		return INSERT;
	}

	return NIL;
}

void cargarPaqueteSelect(tSelect *pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 3, " ");
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")) {
		pack->type = SELECT;
		pack->nombre_tabla = spliteado[1];
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		printf("nombre tabla: %s \n", pack->nombre_tabla);
		printf("long tabla: %d \n", pack->nombre_tabla_long);
		pack->key = atoi(spliteado[2]);
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->key);
	} else {
		printf("no entendi tu consulta\n");
	}
}
void cargarPaqueteInsert(tInsert *pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 4, " ");
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")
			&& strcmp(spliteado[3], "")) {
		pack->type = INSERT;
		pack->nombre_tabla = spliteado[1];
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		printf("nombre tabla: %s \n", pack->nombre_tabla);
		printf("long tabla: %d \n", pack->nombre_tabla_long);
		pack->key = atoi(spliteado[2]);
		pack->value = spliteado[3];
		pack->value_long = strlen(spliteado[3]) + 1;
		printf("value: %s \n", pack->value);
		printf("value long: %d \n", pack->value_long);
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->key)
				+ sizeof(pack->value_long) + pack->value_long;
	} else {
		printf("no entendi tu consulta\n");
	}
}

int despacharQuery(char* consulta, int socket_memoria) {
	char** tempSplit;
	tSelect paqueteSelect;
	tInsert paqueteInsert;
	type typeHeader;
	char* serializado = "";
	int consultaOk = 0;
	tempSplit = string_n_split(consulta, 2, " ");
	if (strcmp(tempSplit[0], "")) {
		typeHeader = validarSegunHeader(tempSplit[0]);
		switch (typeHeader) {
		case SELECT:
			cargarPaqueteSelect(&paqueteSelect, consulta);
			serializado = serializarSelect(&paqueteSelect);
			enviarPaquete(socket_memoria, serializado, paqueteSelect.length);
			consultaOk = 1;
			break;
		case INSERT:
			cargarPaqueteInsert(&paqueteInsert, consulta);
			serializado = serializarInsert(&paqueteInsert);
			enviarPaquete(socket_memoria, serializado, paqueteInsert.length);
			printf("serializo el insert \n");
			consultaOk = 1;
			break;
		default:
			printf("para macho que no se, estoy aprendiendo\n");
			break;
		}

	}
	return consultaOk;
}