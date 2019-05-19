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
	tSelect paqueteSelect;
	int consultaOk = 0;
	char** tempSplit;
	type typeHeader;
	char* serializado = "";
	while (1) {

		fgets(consulta, 256, stdin);
		tempSplit = string_n_split(consulta, 2, " ");
		if (strcmp(tempSplit[0], "")) {
			typeHeader = validarSegunHeader(tempSplit[0]);
			if (typeHeader == SELECT) {
				cargarPaqueteSelect(&paqueteSelect, consulta);
				serializado = serializarSelect(&paqueteSelect);
				consultaOk = 1;
			}
			if (typeHeader == INSERT) {
				printf("mandaste un insert bro \n");
				consultaOk = 1;
			}
		}
		if (!consultaOk) {
			printf("no entendi tu header \n");
		}

		if (strcmp(serializado, "")) {
			enviarPaquete(socket_memoria, serializado, paqueteSelect.length);
		}
		consultaOk=0;
		serializado="";
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

