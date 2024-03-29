/*
 * Querys.c
 *
 *  Created on: 7 jun. 2019
 *      Author: utnso
 */

#include "Querys.h"



void ejecutarConsulta(int socket, type header) {
	encontroSeg = -1;
	indexPag = -1;
	tPagina* pagina = malloc(sizeof(tPagina));
	pagina->value = malloc(tamanioMaxValue);
	tSegmento* miSegmento = malloc(sizeof(tSegmento));
	elem_tabla_pag* pagTabla = malloc(sizeof(elem_tabla_pag));
	encontroSeg = -1;
	indexPag = -1;
	int error;
	switch (header) {
	case SELECT:
		packSelect = malloc(sizeof(tSelect));
		cargarPackSelect(packSelect, leyoConsola, paramsConsola->consulta,socket);
		encontroSeg = buscarSegmentoEnTabla(packSelect->nombre_tabla,
				miSegmento, tablaSegmentos);
		if (encontroSeg == 1) {
			log_debug(logger, "Encontre segmento: %s ",
					packSelect->nombre_tabla);
			indexPag = buscarPaginaEnMemoria(packSelect->key,miSegmento ,pagTabla,pagina);
		}
		tRegistroRespuesta* reg = malloc(sizeof(tRegistroRespuesta));
		reg->value = malloc(tamanioMaxValue);
		if (indexPag >= 0) {
			log_debug(logger, "Encontre pagina buscada");
			reg->tipo = REGISTRO;
			reg->timestamp = pagina->timestamp;
			strcpy(reg->value,pagina->value);
			reg->key = pagina->key;
			reg->value_long = strlen(pagina->value) + 1;
			error = 1;
			log_debug(logger, "El value es: %s", pagina->value);
			log_debug(logger, "El time es es: %llu", pagina->timestamp);
		} else {
			error = pedirRegistroALFS(socket_lfs, packSelect, reg);
			if (reg->timestamp != 0 && strcmp(reg->value, "")) {
				log_debug(logger, "El value es: %s", reg->value);
				log_debug(logger, "El time es es: %llu", reg->timestamp);
				pagina->key = reg->key;
				pagina->timestamp = reg->timestamp;
				strcpy(pagina->value, reg->value);
				if (encontroSeg != 1) {
					cargarSegmentoEnTabla(packSelect->nombre_tabla,
							tablaSegmentos);
					miSegmento = obtenerUltimoSegmentoDeTabla(tablaSegmentos);
				}
				error = agregarPaginaAMemoria(miSegmento, pagina,false);
			}else{
				error = 1;
				log_error(logger,"No existe el registro");
			}
		}
		validarAgregadoDePagina(leyoConsola, &error ,socket, miSegmento, pagina, false);
		if(error == 1){
			enviarRegistroAKernel(reg, socket, leyoConsola);
		}
		free(reg->value);
		free(reg);
		free(packSelect->nombre_tabla);
		free(packSelect);

		break;
	case INSERT:
		packInsert = malloc(sizeof(tInsert));
		error = 1;
		cargarPackInsert(packInsert, leyoConsola, paramsConsola->consulta, socket);
		encontroSeg = buscarSegmentoEnTabla(packInsert->nombre_tabla,
				miSegmento, tablaSegmentos);
		if (encontroSeg == 1) {
			log_debug(logger, "Encontre segmento %s ",
					packInsert->nombre_tabla);

			indexPag = buscarPaginaEnMemoria(packInsert->key, miSegmento,
					pagTabla,pagina);
			if (indexPag >= 0) {
				log_debug(logger,
						"Encontre la pagina buscada en el segmento %s ",
						packInsert->nombre_tabla);
				error = actualizarPaginaEnMemoria(miSegmento, indexPag, packInsert->value);


			} else {
				log_debug(logger,
						"No se encontro la pagina en memoria");
				pagina->key = packInsert->key;
				pagina->timestamp = obtenerTimestamp();
				strcpy(pagina->value,packInsert->value);
				error = agregarPaginaAMemoria(miSegmento,pagina,true);

			}

		} else {
			//No encontro el segmento en tabla de segmentos
			log_debug(logger, "No encontro el segmento en tabla de segmentos");
			pagina->key = packInsert->key;
			pagina->timestamp = obtenerTimestamp();
			strcpy(pagina->value, packInsert->value);
			cargarSegmentoEnTabla(packInsert->nombre_tabla, tablaSegmentos);
			miSegmento = obtenerUltimoSegmentoDeTabla(tablaSegmentos);
			error = agregarPaginaAMemoria(miSegmento,pagina,true);

		}

		validarAgregadoDePagina(leyoConsola,&error,socket, miSegmento, pagina,true);
		free(packInsert->nombre_tabla);
		free(packInsert->value);
		free(packInsert);



		break;
	case CREATE:
		packCreate = malloc(sizeof(tCreate));
		cargarPackCreate(packCreate, leyoConsola, paramsConsola->consulta, socket);
		char* createAEnviar = serializarCreate(packCreate);
		enviarPaquete(socket_lfs, createAEnviar, packCreate->length);
		log_debug(logger, "Envio la consulta a LFS");
		free(packCreate->consistencia);
		free(packCreate->nombre_tabla);
		free(packCreate);
		free(createAEnviar);
		break;

	case DROP:
		packDrop = malloc(sizeof(tDrop));
		cargarPackDrop(packDrop, leyoConsola, paramsConsola->consulta, socket);
		packDrop->type = DROP;
		log_debug(logger, "Drop Tabla: %s", packDrop->nombre_tabla);
		encontroSeg = buscarSegmentoEnTabla(packDrop->nombre_tabla, miSegmento, tablaSegmentos);
		if (encontroSeg == 1) {
			log_debug(logger, "Encontre segmento: %s", packDrop->nombre_tabla);
			liberarPaginasDelSegmento(miSegmento, tablaSegmentos);
			log_debug(logger, "Segmento eliminado");
		} else {
			log_error(logger, "No se encontro el segmento");
		}
		char* dropSerializado = serializarDrop(packDrop);
		enviarPaquete(socket_lfs, dropSerializado, packDrop->length);
		log_debug(logger, "Envio DROP a LFS");
		free(packDrop->nombre_tabla);
		free(packDrop);
		free(dropSerializado);
		break;

	case JOURNAL:
		packJournal = malloc(sizeof(tJournal));
		cargarPackJournal(packJournal, leyoConsola, paramsConsola->consulta, socket);
		ejecutarJournal();
		break;

	case DESCRIBE:
		log_debug(logger,"Llega un Describe");
		packDescribe = malloc(sizeof(tDescribe));
		packDescResp = malloc(sizeof(t_describe));
		desSerializarDescribe(packDescribe, socket);
		char* serializado = serializarDescribe(packDescribe);
		int bytes = 0;
		enviarPaquete(socket_lfs, serializado, packDescribe->length);
		bytes = desserializarDescribe_Response(packDescResp, socket_lfs);
		while(bytes <= 0){
			close(socket_lfs);
			reconectarLFS();
			enviarPaquete(socket_lfs, serializado, packDescribe->length);
			bytes = desserializarDescribe_Response(packDescResp, socket_lfs);
		}
		if (packDescResp->cant_tablas == 0) {
			t_describe* describe = malloc(sizeof(t_describe));
			describe->cant_tablas = 0;
			char* serializedPackage;
			serializedPackage = serializarDescribe_Response(describe);
			send(socket, serializedPackage, sizeof(describe->cant_tablas),
					0);
			dispose_package(&serializedPackage);
		} else {
			char* respSerializada = serializarDescribe_Response(
					packDescResp);
			int length = packDescResp->cant_tablas * sizeof(t_metadata)
					+ sizeof(uint16_t);
			enviarPaquete(socket, respSerializada, length);
			//free(respSerializada);
		}


		free(packDescResp->tablas);

		free(serializado);
		free(packDescResp);
		free(packDescribe->nombre_tabla);
		free(packDescribe);
		break;
	case GOSSIPING:
		packGossip = malloc(sizeof(tGossip));
		desSerializarGossip(packGossip,socket);
		actualizarTablaGossip(packGossip);
		log_debug(logger,"Cantidad de memorias que conozco: %d",tablaGossip->elements_count);
		tGossip* gossipResp = malloc(sizeof(tGossip));
		gossipResp->memorias = malloc(tablaGossip->elements_count * sizeof(tMemoria));
		//FD_CLR(socket,&active_fd_set);
		//close(socket);
		//int socketResp = levantarCliente(packGossip->memorias[0].puerto , packGossip->memorias[0].ip);
		devolverTablaGossip(gossipResp,socket);
		free(packGossip->memorias);
		free(gossipResp->memorias);
		free(packGossip);
		free(gossipResp);

		break;
	case RESPGOSS:
		packGossip = malloc(sizeof(tGossip));
		desSerializarGossip(packGossip,socket);
		actualizarTablaGossip(packGossip);
		//FD_CLR(socket,&active_fd_set);
		//close(socket);
		free(packGossip->memorias);
		free(packGossip);

		log_debug(logger,"Cantidad de memorias que conozco: %d",tablaGossip->elements_count);
		/*
		desSerializarGossiping(RESPGOSS);
		actualizarTablaGossip(tablaGossip);
		*/
		break;
	case GOSSIPKERNEL:
		log_debug(logger,"Llego gossip kernel");
		gossipKernel = malloc(sizeof(tGossip));
		gossipKernel->memorias = malloc(
				tablaGossip->elements_count * sizeof(tMemoria));
		devolverTablaGossip(gossipKernel, socket);
		free(gossipKernel->memorias);
		free(gossipKernel);

		break;
	case SIGNAL:
		log_debug(logger,"Kernel quiere saber si estoy vivo");
		send(socket,&miConfig->numeroMemoria,sizeof(int),0);
		break;
	case NIL:
		log_error(logger, "No entendi la consulta");
		break;
	}
	free(miSegmento);
	free(pagina->value);
	free(pagina);
	free(pagTabla);

}

int pedirRegistroALFS(int socket, tSelect* packSelect, tRegistroRespuesta* reg) {
	usleep(miConfig->retardoFS * 1000);
	log_debug(logger, "Pido registro a LFS");
	char* selectAEnviar = serializarSelect(packSelect);
	int bytes = enviarPaquete(socket, selectAEnviar, packSelect->length);
	if (bytes > 0) {
		type header;
		int error = recv(socket, &header, sizeof(type), MSG_WAITALL);
		if(0 >= error){
			reconectarLFS();
			//bytes = enviarPaquete(socket_lfs, selectAEnviar, packSelect->length);
			//error = recv(socket_lfs, &header, sizeof(type), MSG_WAITALL);
			//log_warning(logger,"error: %d",error);
			pedirRegistroALFS(socket_lfs, packSelect, reg);
		}
		if (header == REGISTRO) {
			desSerializarRegistro(reg, socket);
			reg->tipo = REGISTRO;
			free(selectAEnviar);
			return 1;
		} else {
			return -3;
		}
	}
	return -3;
}

void enviarRegistroAKernel(tRegistroRespuesta* reg, int socket,
		bool leyoConsola) {
	//SI LEYO DE CONSOLA NO QUIERO ENVIARSELO A KERNEL
	if (!leyoConsola) {
		char* registroSerializado = serializarRegistro(reg);
		enviarPaquete(socket, registroSerializado, reg->length);
		log_debug(logger, "Value enviado a Kernel");
	}
}

void* leerQuery(void* params) {
	while (1) {
		tHiloConsola* parametros = (tHiloConsola*) params;
		leyoConsola = false;
		bool consultaOk = false;
		type head;
		fgets(parametros->consulta, 256, stdin);
		char** tempSplit;
		tempSplit = string_n_split(parametros->consulta, 2, " ");
		string_to_upper(tempSplit[0]);
		if (!strcmp(tempSplit[0], "SELECT")) {
			log_debug(logger,"llega SELECT");
			if (validarSelect(parametros->consulta)) {
				head = SELECT;
				consultaOk = true;
			}
		}
		if (!strcmp(tempSplit[0], "INSERT")) {
			log_debug(logger,"llega INSERT");
			if(validarInsert(parametros->consulta)){
				head = INSERT;
				consultaOk = true;
			}

		}
		if (!strcmp(tempSplit[0], "CREATE")) {
			log_debug(logger,"llega CREATE");
			if(validarCreate(parametros->consulta)){
				head = CREATE;
				consultaOk = true;
			}

		}
		if(!strcmp(tempSplit[0], "DROP")){
			log_debug(logger,"llega DROP");
			if(strcmp(tempSplit[1],"")){
				head = DROP;
				consultaOk = true;
			}
		}
		if(!strcmp(tempSplit[0], "JOURNAL\n")){
			log_debug(logger,"llega JOURNAL");
			head = JOURNAL;
			consultaOk = true;
		}
		leyoConsola = true;
		free(tempSplit[0]);
		free(tempSplit[1]);
		free(tempSplit);
		if(consultaOk){
			sem_wait(&mutexJournal);
			ejecutarConsulta(-1, head);
			sem_post(&mutexJournal);
		}else{
			log_error(logger,"Error de sintaxis en la consulta");
		}


	}
}

void journalAsincronico(){
	while (1) {
		usleep(miConfig->retardoJournal * 1000);
		sem_wait(&mutexJournal);
		ejecutarJournal();
		sem_post(&mutexJournal);
	}
}

void reconectarLFS(){
	int conexion = 0;
	log_error(logger,"Error de conexion con LFS");
	while(!conexion){
		socket_lfs = levantarCliente((char*) miConfig->puerto_fs,
				miConfig->ip_fs);
			tamanioMaxValue = handshakeLFS(socket_lfs);
			log_debug(logger,
					"Handshake con LFS realizado. Tamanio max del value: %d",
					tamanioMaxValue);
			log_debug(logger, "Levanta conexion con LFS");
			conexion = 1;
			setsockopt(socket_lfs,SOL_SOCKET,SO_REUSEADDR,&conexion,sizeof(int));

	}

}

void cargarPackSelect(tSelect* packSelect,bool leyoConsola,char consulta[], int socket){
	if(leyoConsola){
		cargarPaqueteSelect(packSelect, consulta);
	}else{
		desSerializarSelect(packSelect, socket);
	}

}

void cargarPackInsert(tInsert* packInsert, bool leyoConsola, char consulta[], int socket) {
	if (leyoConsola) {
		cargarPaqueteInsert(packInsert, consulta);
	} else {
		desSerializarInsert(packInsert, socket);
	}

}

void cargarPackCreate(tCreate* packCreate,bool leyoConsola,char consulta[], int socket){
	if(leyoConsola){
		cargarPaqueteCreate(packCreate, consulta);
	}else{
		desSerializarCreate(packCreate, socket);
	}

}
void cargarPackDrop(tDrop* packDrop, bool leyoConsola, char consulta[], int socket){
	if(leyoConsola){
		cargarPaqueteDrop(packDrop, consulta);
	} else {
		desSerializarDrop(packDrop, socket);
	}
}


void cargarPackJournal(tJournal* packJournal, bool leyoConsola, char consulta[], int socket){
	if(leyoConsola){
		cargarPaqueteJournal(packJournal, consulta);
	} else {
		desSerializarJournal(packJournal, socket);
	}
}
int handshakeLFS(int socket_lfs){
	int buffer;
	recv(socket_lfs,&buffer,4,MSG_WAITALL);
	return buffer;
}

void innotificar() {
	log_debug(logger, "entre al hilo");
	int length;

	while (1) {
		char buffer[BUF_LEN];

		int file_descriptor = inotify_init();
		if (file_descriptor < 0) {
			perror("inotify_init");
		}

		int watch_descriptor = inotify_add_watch(file_descriptor, "../Debug", IN_MODIFY);

		length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}

		int offset = 0;

		while (offset < length) {

			struct inotify_event *event = (struct inotify_event *) &buffer[offset];

			if (event->len) {

				if (event->mask & IN_CREATE) {
					if (event->mask & IN_ISDIR) {
						log_debug(logger,"El directorio %s se creo", event->name);
					} else {
						log_debug(logger,"El archivo %s se creo", event->name);
					}
				} else if (event->mask & IN_DELETE) {
					if (event->mask & IN_ISDIR) {
						log_debug(logger,"El directorio %s se elimino", event->name);
					} else {
						log_debug(logger,"El archivo %s se elimino", event->name);
					}
				} else if (event->mask & IN_MODIFY) {
					if (event->mask & IN_ISDIR) {
						log_debug(logger,"El directorio %s se modifico", event->name);
					} else {
						log_debug(logger,"El archivo %s se modifico", event->name);
						config = config_create(pathConfig);
						cargarConfig();
					}
				}

			}
			offset += sizeof (struct inotify_event) + event->len;
		}

		inotify_rm_watch(file_descriptor, watch_descriptor);
		close(file_descriptor);
	}
}
