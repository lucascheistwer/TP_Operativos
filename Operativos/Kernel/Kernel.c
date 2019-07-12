/*
 * Kernel.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "Kernel.h"

#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

char package[PACKAGESIZE];
struct addrinfo hints;
struct addrinfo *serverInfo;
sem_t mutexSocket;
sem_t semReady;
sem_t mutexReady;
sem_t mutexNew;
sem_t semNew;
sem_t mutexSC;
sem_t mutexSHC;
sem_t mutexEC;
pthread_t planificador_t;
t_log *logger;
t_log *loggerError;
t_queue *colaReady;
t_queue *colaNew;
t_infoMem *SC;
t_list *listaMemsEC;
t_list *listaMemsSHC;
t_list *mems;
t_list *listaTablas;
configKernel *miConfig;
t_config* config;
int contScript;
bool continuar = true;

// Define cual va a ser el size maximo del paquete a enviar

int main(){
	signal(SIGINT, finalizarEjecucion);
	inicializarTodo();
	int socket_memoria = levantarCliente(miConfig->puerto_mem, miConfig->ip_mem);
	log_debug(logger,"Sockets inicializados con exito");
	log_debug(logger,"Se tendra un nivel de multiprocesamiento de: %d cpus", miConfig->MULT_PROC);
	pthread_t cpus[miConfig->MULT_PROC];
	despacharQuery("describe\n",socket_memoria);
	if(levantarCpus(socket_memoria,cpus)){
		colaReady = queue_create();
		colaNew = queue_create();
		pthread_create(&planificador_t, NULL, (void*) planificador, NULL);
		char consulta[256] = "";
		for(int i = 0; i < miConfig->MULT_PROC; i++){
					pthread_detach(cpus[i]);
				}
				pthread_detach(planificador_t);
		while (continuar) {

			fgets(consulta, 256, stdin);
			int consultaOk = despacharQuery(consulta, socket_memoria);
			if (!consultaOk) {
				printf("no entendi tu header \n");
				log_error(loggerError,"Se ingreso un header no valido");
			}
			consultaOk = 0;
		}

	}else{
		log_error(loggerError,"No se han podido levantar las cpus...");
	}
	/*sem_destroy(&semReady);
	sem_destroy(&semNew);
	sem_destroy(&mutexNew);
	sem_destroy(&mutexReady);
	sem_destroy(&mutexSocket);*/
	close(socket_memoria);
}

type validarSegunHeader(char* header) {
	string_to_upper(header);
	if (!strcmp(header, "SELECT")) {
		return SELECT;
	}
	if (!strcmp(header, "INSERT")) {
		return INSERT;
	}
	if (!strcmp(header, "RUN")) {
			return RUN;
	}
	if (!strcmp(header, "CREATE")) {
				return CREATE;
		}
	if (!strcmp(header, "ADD")) {
					return ADD;
			}

	if (!strcmp(header, "DESCRIBE")) {
					return DESCRIBE;
			}
	if (!strcmp(header, "DESCRIBE\n")) {
					return DESCRIBE;
			}
	if (!strcmp(header, "JOURNAL\n")){
					return JOURNAL;
			}
	if (!strcmp(header, "DROP")){
						return DROP;
				}
	return NIL;
}






int despacharQuery(char* consulta, int socket_memoria) {
	char** tempSplit;
	tSelect* paqueteSelect=malloc(sizeof(tSelect));
	tInsert* paqueteInsert=malloc(sizeof(tInsert));
	tCreate* paqueteCreate = malloc(sizeof(tCreate));
	tJournal* paqueteJournal = malloc(sizeof(tJournal));
	tDrop* paqueteDrop = malloc(sizeof(tDrop));
	tDescribe* paqueteDescribe = malloc(sizeof(tDescribe));
	type typeHeader;
	char* serializado = "";
	int consultaOk = 0;
	tempSplit = string_n_split(consulta, 2, " ");
	log_debug(logger, "header %s", tempSplit[0]);
	script *unScript;
	if (strcmp(tempSplit[0], "")) {
		typeHeader = validarSegunHeader(tempSplit[0]);
		switch (typeHeader) {
		case SELECT:
			if(validarSelect(consulta)){
				int error;
				log_debug(logger,"Se recibio un SELECT");
				cargarPaqueteSelect(paqueteSelect, consulta);
				serializado = serializarSelect(paqueteSelect);
				sem_wait(&mutexSocket);
				enviarPaquete(socket_memoria, serializado, paqueteSelect->length);
				recv(socket_memoria, &error, sizeof(error), 0);
				if (error == -1) {
					log_error(logger, "Memoria llena, hago JOURNAL");
					cargarPaqueteJournal(paqueteJournal, "JOURNAL");
					serializado = serializarJournal(paqueteJournal);
					sem_wait(&mutexSocket);
					enviarPaquete(socket_memoria, serializado,
							paqueteJournal->length);
					sem_post(&mutexSocket);
					serializado = serializarSelect(paqueteSelect);
					sem_wait(&mutexSocket);
					enviarPaquete(socket_memoria, serializado,
							paqueteSelect->length);
					sem_post(&mutexSocket);
					recv(socket_memoria, &error, sizeof(error), 0);
					consultaOk = 1;
				}
				type header = leerHeader(socket_memoria);
				tRegistroRespuesta* reg = malloc(sizeof(tRegistroRespuesta));
				desSerializarRegistro(reg,socket_memoria);
				log_debug(logger,"Value: %s",reg->value);
				sem_post(&mutexSocket);
				free(paqueteSelect->nombre_tabla);
				free(reg->value);
				free(reg);
				free(serializado);
			}else{
				printf("Por favor ingrese la consulta en formato correcto \n");
				log_error(loggerError,"Se ingreso una consulta no valida");
			}
			consultaOk = 1;
			break;
		case INSERT:
			if(validarInsert(consulta)){
				int error = 0;
				log_debug(logger,"Se recibio un INSERT");
				char* sinFin = string_substring_until(consulta,string_length(consulta)-1 );
				cargarPaqueteInsert(paqueteInsert, sinFin);
				serializado = serializarInsert(paqueteInsert);
				log_debug(logger,"%s",paqueteInsert->value);
				sem_wait(&mutexSocket);
				enviarPaquete(socket_memoria, serializado, paqueteInsert->length);
				sem_post(&mutexSocket);
				recv(socket_memoria, &error, sizeof(error), 0);
				if(error == 1){
					log_debug(logger, "Se inserto el valor: %s", paqueteInsert->value);
				} else {
					log_error(logger, "Memoria llena, hago JOURNAL");
					cargarPaqueteJournal(paqueteJournal, "JOURNAL");
					serializado = serializarJournal(paqueteJournal);
					sem_wait(&mutexSocket);
					enviarPaquete(socket_memoria, serializado,
							paqueteJournal->length);
					sem_post(&mutexSocket);
					serializado = serializarInsert(paqueteInsert);
					sem_wait(&mutexSocket);
					enviarPaquete(socket_memoria, serializado, paqueteInsert->length);
					sem_post(&mutexSocket);
					recv(socket_memoria, &error, sizeof(error), 0);
					consultaOk = 1;
				}
				free(serializado);
				free(paqueteInsert->nombre_tabla);
				free(paqueteInsert->value);
				free(sinFin);
			}else{
				printf("Por favor ingrese la consulta en formato correcto \n");
				log_error(loggerError,"Se ingreso una consulta no valida");
			}
			consultaOk = 1;
			break;
		case CREATE:
			if(validarCreate(consulta)){
				log_debug(logger,"Se recibio un CREATE");
				char* sinFin = string_substring_until(consulta,string_length(consulta)-1 );
				cargarPaqueteCreate(paqueteCreate,sinFin);
				serializado = serializarCreate(paqueteCreate);
				sem_wait(&mutexSocket);
				enviarPaquete(socket_memoria, serializado, paqueteCreate->length);
				sem_post(&mutexSocket);
				free(serializado);
				free(paqueteCreate->consistencia);
				free(paqueteCreate->nombre_tabla);
				free(sinFin);
			}else{
				printf("Por favor ingrese la consulta en formato correcto \n");
				log_error(loggerError,"Se ingreso una consulta no valida");
			}
			consultaOk = 1;
			break;
		case RUN:
			log_debug(logger,"Se recibio un RUN con la siguiente path: %s", tempSplit[1]);
			char* sinFin = string_substring_until(tempSplit[1],string_length(tempSplit[1])-1 );
			unScript = malloc(sizeof(script));
			unScript->path = sinFin;
			unScript->pos = 0;
			unScript->estado = new;
			unScript->id = contScript;
			contScript++;
			sem_wait(&mutexNew);
			queue_push(colaNew, unScript);
			log_debug(logger,"Se cargo a la cola de new el script id: %d", unScript->id);
			sem_post(&semNew);
			sem_post(&mutexNew);
			consultaOk = 1;
			break;
		case ADD:
			if(validarAdd(consulta)){
				log_debug(logger,"Se recibio un ADD");
				ejecutarAdd(consulta);
				//log_debug(logger,"Se agrego el criterio a: %d ", ((t_infoMem*) list_get(listaMems, 0))->id);
			}
			consultaOk = 1;
			break;
		case DESCRIBE:
			log_debug(logger,"Se recibio un DESCRIBE");
			cargarPaqueteDescribe(paqueteDescribe,
					string_substring_until(consulta,string_length(consulta)-1  ) );
			serializado = serializarDescribe(paqueteDescribe);
			enviarPaquete(socket_memoria,serializado,paqueteDescribe->length);
			//type header = leerHeader(socket_memoria);
			t_describe* response = malloc(sizeof(t_describe));
			desserializarDescribe_Response(response,socket_memoria);
			log_debug(logger,"Cant tablas: %d", response->cant_tablas);
			ejecutarDescribe(response);
			free(serializado);
			free(paqueteDescribe->nombre_tabla);
			consultaOk = 1;
			break;
		case JOURNAL:
			log_debug(logger,"Se recibio un JOURNAL");
			cargarPaqueteJournal(paqueteJournal,
					string_substring_until(consulta,string_length(consulta)-1  ) );
			serializado = serializarJournal(paqueteJournal);
			sem_wait(&mutexSocket);
			enviarPaquete(socket_memoria, serializado, paqueteJournal->length);
			sem_post(&mutexSocket);
			free(serializado);
			consultaOk = 1;
			break;
		case DROP:
			log_debug(logger, "Se recibio un DROP");
			cargarPaqueteDrop(paqueteDrop,
					string_substring_until(consulta,
							string_length(consulta) - 1));
			serializado = serializarDrop(paqueteDrop);
			sem_wait(&mutexSocket);
			enviarPaquete(socket_memoria, serializado, paqueteJournal->length);
			sem_post(&mutexSocket);
			consultaOk = 1;
			free(serializado);
			free(paqueteDrop->nombre_tabla);
			break;
		default:
			printf("Operacion no valida por el momento \n");
			break;
		}

	}
	free(paqueteCreate);
	free(paqueteInsert);
	free(paqueteSelect);
	free(paqueteDescribe);
	free(paqueteDrop);
	free(paqueteJournal);
	string_iterate_lines(tempSplit,free);
	free(tempSplit);
	return consultaOk;
}

void CPU(int socket_memoria){
	/*
	 * QUERIDOS CHOLO Y FELIPE
	 * SEGUN VI EN VARIOS POST DE STACK OVERFLOW
	 * NO HABRIA PROBLEMA SI ME CONECTO AL MISMO PUERTO
	 * (CUANDO TENGAMOS MAS MEMORIAS MEPA QUE VA A HABER QUE USAR UN PUERTO DIF PARA
	 * CADA UNA)
	 * SI ESTO NO VA TENEMOS QUE VER DE HACER UN HANDSHAKE Y QUE EN BASE AL MULTIPROCESAMIENTO
	 * USTEDES ME DEVUELVAN UN ARRAY DE PUERTOS.
	 * CON AMOR... UNTER.
	 */
	 socket_memoria = levantarCliente(miConfig->puerto_mem,miConfig->ip_mem);

	while(continuar){
		script *unScript;
		char* consulta = malloc(256);
		sem_wait(&semReady);
		sem_wait(&mutexReady);
		unScript = ( script *) queue_pop(colaReady);
		sem_post(&mutexReady);
		unScript->estado = exec;
		int info;
		log_debug(logger,"Se esta corriendo el script: %d", unScript->id);
		for(int i = 0 ; i < miConfig->quantum; i++){
			info = leerLinea(unScript->path,unScript->pos,consulta);
			switch(info){
			case 0:
				i = miConfig->quantum;
				unScript->estado = exit_;
				log_error(loggerError,"El script rompio en la linea: %d",
						unScript->pos);
				free(unScript->path);
				free(unScript);
				break;
			case 1:
				log_debug(logger,"Enviando linea %d", unScript->pos);
				info = despacharQuery(consulta,socket_memoria);
				if(info!=1){
					i = miConfig->quantum;
					unScript->estado = exit_;
					log_error(loggerError,"El script rompio en la linea: %d",
							unScript->pos);
					free(unScript->path);
					free(unScript);
				}else{
					unScript->pos++;
				}
				break;
			case 2:
				unScript->estado = exit_;
				log_debug(logger,
						"El script id: %d finalizo con exito en la linea: %d",
						unScript->id,
						unScript->pos);
				i = miConfig->quantum;
				free(unScript->path);
				free(unScript);
			}
		}
		if(info == 1){
			sem_wait(&mutexReady);
			queue_push(colaReady,unScript);
			log_debug(logger,"el script con id: %d ha vuelto a ready",
					unScript->id);
			sem_post(&semReady);
			sem_post(&mutexReady);
		}
		free(consulta);
	}
	close(socket_memoria);
}

void planificador(){
	while(1){
		script *unScript;
		sem_wait(&semNew);
		sem_wait(&mutexNew);
		unScript = ( script *) queue_pop(colaNew);
		sem_post(&mutexNew);
		unScript->estado = ready;
		sem_wait(&mutexReady);
		queue_push(colaReady, unScript);
		log_debug(logger,"Se cargo a la cola de ready el run con id: %d",
				unScript->id);
		sem_post(&semReady);
		sem_post(&mutexReady);
	}
}

int levantarCpus(int socket_memoria, pthread_t cpus[]){
	int devolver = 1;
	for(int i = 0; i < miConfig->MULT_PROC; i++){
		devolver += pthread_create(&cpus[i], NULL, (void*) CPU, (int *) socket_memoria);
	}
	return devolver;
}

int leerLinea(char* path, int linea, char* leido){
	log_debug(logger,"%s", path);
	FILE* archivo = fopen(path,"r");
	if(archivo != NULL){
		int cont = 0;
		while (fgets(leido, 256, archivo) != NULL)
		{
			if (cont == linea)
			{
				fclose(archivo);
				return 1; //Salio bien
			}
			else
			{
				cont++;
			}
		}
	}
	else{
		printf("El archivo no existe... \n");
		return 0; //devuelvo 0 si es error
	}
	if(feof(archivo)){
		fclose(archivo);
		return 2; //devuelvo 2 si llegue al fin del archivo
	}
	printf("Algo salio mal \n");
	return 0; //devuelve 0 si no se encontro la linea

}
int validarSelect(char* consulta){
	char** split = string_split(consulta," ");
	int i = 0;
	while(split[i] != NULL){
		i++;
	}
	string_iterate_lines(split,free);
	free(split);
	return 3 == i;
}
int validarInsert(char* consulta){
	char** value = string_split(consulta,"\"");
	char** split = string_split(value[0]," ");
	int i = 0;
	while(split[i] != NULL){
		i++;
	}
	int j = 0;
	while(value[j] != NULL){
			j++;
		}
	string_iterate_lines(split,free);
	free(split);
	string_iterate_lines(value,free);
	free(value);
	return ((3 == i) && (3 == j)) ;
}
int validarCreate(char* consulta){
	char** split = string_split(consulta," ");
	int i = 0;
	while(split[i] != NULL){
		i++;
	}
	string_iterate_lines(split,free);
	free(split);
	return 5 == i;
}

int validarAdd(char* consulta){
	char** split = string_split(consulta," ");
	bool sintaxis = true;
	int i = 0;
	while(split[i] != NULL && sintaxis){
		string_to_upper(split[i]);
		switch (i){
		case 1:
			if(strcmp(split[i],"MEMORY")){
				sintaxis = false;
			}
			break;
		case 3:
			if(strcmp(split[i],"TO")){
				sintaxis = false;
			}
			break;
		}
		i++;
	}
	string_iterate_lines(split,free);
	free(split);
	return ((5 == i) && sintaxis);
}

void ejecutarAdd(char* consulta){
	char** split = string_split(consulta," ");
	t_infoMem* memAdd = generarMem(consulta);
	bool mismoId(void* elemento) {
			t_infoMem* mem = malloc(sizeof(t_infoMem));
			mem = (t_infoMem*) elemento;
			int id = mem->id;
			return (atoi(split[2]) == id);
		}
	consistencias cons = obtCons(split[4]);
	switch (cons){
	case sc:
		sem_wait(&mutexSC);
		SC = memAdd;
		log_debug(logger,"La memoria %d se asoció al criterio %s",memAdd->id,split[4]);
		sem_post(&mutexSC);
		break;
	case shc:
		sem_wait(&mutexSHC);
		if(list_is_empty(list_filter(listaMemsSHC, mismoId))){
						list_add(listaMemsSHC, memAdd);
						log_debug(logger,"La memoria %d se asoció al criterio %s",memAdd->id,split[4]);
					}else{
						log_error(loggerError,"Ya se encuentra asociada la memoria %d al criterio",memAdd->id);
					}
		sem_post(&mutexSHC);
		//despacharQuery("journal\n",)
		break;
	case ec:
		sem_wait(&mutexEC);
		if(list_is_empty(list_filter(listaMemsEC, mismoId))){
						list_add(listaMemsEC, memAdd);
						log_debug(logger,"La memoria %d se asoció al criterio %s",memAdd->id,split[4]);
					}else{
						log_error(loggerError,"Ya se encuentra asociada la memoria %d al criterio",memAdd->id);
					}
		sem_post(&mutexEC);
		break;
	}
	string_iterate_lines(split,free);
	free(split);
}

t_infoMem* generarMem(char* consulta){
	char** split = string_split(consulta," ");
	t_infoMem* mem = malloc(sizeof(t_infoMem));
	mem->id = atoi(split[2]);
	return mem;
}

consistencias obtCons(char* criterio){
	string_to_upper(criterio);
	if(!strcmp(criterio,"SC\n")){
		return sc;
	}
	if(!strcmp(criterio,"SHC\n")){
			return shc;
		}
	if(!strcmp(criterio,"EC\n")){
			return ec;
		}
}

void ejecutarDescribe(t_describe *response){

	for(int i = 0; i < response->cant_tablas; i++){
		t_metadata *metadata = malloc(sizeof(t_metadata));
		memcpy(metadata,&(response->tablas[i]),sizeof(t_metadata));
		char nombre[12];
		strcpy(nombre, metadata->nombre_tabla);
		bool mismoNombre(void* elemento) {
						t_metadata* tabla = malloc(sizeof(t_metadata));
						tabla = (t_metadata*) elemento;
						return (!strcmp(tabla->nombre_tabla,nombre));
					}
		if(list_is_empty(list_filter(listaTablas,mismoNombre))){
			list_add(listaTablas,metadata);
			log_debug(logger, "Se agrego la tabla %s a la lista", nombre);
		}
	}
}
consistencias consTabla (char* nombre){
	bool mismoNombre(void* elemento) {
		t_metadata* tabla = malloc(sizeof(t_metadata));
		tabla = (t_metadata*) elemento;
		return (!strcmp(tabla->nombre_tabla,nombre));
	}
	t_metadata *tabla = list_find(listaTablas,mismoNombre);
	return obtCons(tabla->consistencia);
}

int SHC(int key){
	sem_wait(&mutexSHC);
	int tamanio = list_size(listaMemsSHC);
	sem_post(&mutexSHC);
	return (tamanio % key);
}

int EC(int time){
	sem_wait(&mutexEC);
	int tamanio = list_size(listaMemsEC);
	sem_post(&mutexEC);
	return (tamanio % time);
}
void cargarConfig(t_config* config){
	miConfig = malloc(sizeof(configKernel));
	miConfig->ip_mem = config_get_string_value(config,"IP");
	miConfig->metadata_refresh = config_get_int_value(config, "METADATA_REFRESH");
	miConfig->MULT_PROC = config_get_int_value(config, "MULTIPROCESAMIENTO");
	miConfig->puerto_mem = config_get_string_value(config,"PUERTO_MEMORIA");
	miConfig->sleep = config_get_int_value(config,"SLEEP_EJECUCION");
	miConfig->quantum = config_get_int_value(config,"QUANTUM");
}

void inicializarTodo(){
	contScript = 1;
	logger = log_create("Kernel.log","Kernel.c",true,LOG_LEVEL_DEBUG);
	loggerError = log_create("Kernel.log","Kernel.c",true,LOG_LEVEL_ERROR);
	log_debug(logger,"Cargando configuracion");
	config = config_create("Kernel.config");
	cargarConfig(config);
	log_debug(logger,"Configuracion cargada con exito");
	log_debug(logger,"Inicializando semaforos");
	sem_init(&semReady,0,0);
	sem_init(&semNew,0,0);
	sem_init(&mutexNew,0,1);
	sem_init(&mutexReady,0,1);
	sem_init(&mutexSocket,0,1);
	sem_init(&mutexEC,0,1);
	sem_init(&mutexSC,0,1);
	sem_init(&mutexSHC,0,1);
	listaMemsEC = list_create();
	listaMemsSHC = list_create();
	listaTablas = list_create();
	log_debug(logger,"Semaforos incializados con exito");
	log_debug(logger,"Inicializando sockets");
}

void finalizarEjecucion() {
	printf("------------------------\n");
	printf("¿chau chau adios?\n");
	printf("------------------------\n");
	log_destroy(logger);
	log_destroy(loggerError);
	free(SC);
	list_iterate(listaMemsEC,free);
	list_iterate(listaMemsSHC,free);
	continuar = false;
	//close(socket_memoria);
	raise(SIGTERM);
}


