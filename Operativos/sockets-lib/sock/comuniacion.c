#include "comunicacion.h"

int enviarPaquete(int clienteSocket, char* payload, uint32_t size) {
	int socket_OK = 1;
	void sigpipe_handler()
	{
	    printf("SIGPIPE caught\n");
	    socket_OK= -1;
	}
    signal(SIGPIPE,sigpipe_handler);
    if(socket_OK == -1){
    	return socket_OK;
    }
	return send(clienteSocket, payload, size, 0);
}

type leerHeader(int socket) {
	type header;
	recv(socket, &header, sizeof(type), MSG_WAITALL);
	return header;
}

unsigned long long obtenerTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long) (tv.tv_sec)) * 1000
			+ ((unsigned long long) (tv.tv_usec)) / 1000;
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

char* serializarGossip(tGossip* packGossip) {
	char* serializedPackage = malloc(sizeof(type) +
			sizeof(packGossip->cant_memorias)
					+ packGossip->cant_memorias * sizeof(tMemoria));
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(type);
	memcpy(serializedPackage + offset, &(packGossip->header),size_to_send);
	offset+= size_to_send;

	size_to_send = sizeof(int);
	memcpy(serializedPackage + offset,&(packGossip->cant_memorias), size_to_send);
	offset+= size_to_send;

	for(int x =0;x<packGossip->cant_memorias;x++){
		size_to_send = sizeof(tMemoria);
		memcpy(serializedPackage + offset, &(packGossip->memorias[x]),size_to_send);
		offset += size_to_send;
	}

	return serializedPackage;
}

int desSerializarGossip(tGossip* packGossip, int socket){


	int status = recv(socket,&(packGossip->cant_memorias),sizeof(int),0);
	if(!status){
		return 0;
	}
	if(status == -1)
		return -1;
	packGossip->memorias = malloc(packGossip->cant_memorias * sizeof(tMemoria));
	for(int x = 0; x< packGossip->cant_memorias; x++){
		status = recv(socket,&(packGossip->memorias[x]),sizeof(tMemoria),0);
	}
	return status;
}

void cargarPaqueteSelect(tSelect *pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 3, " ");
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")) {
		pack->nombre_tabla = malloc(strlen(spliteado[1]) + 1);
		pack->type = SELECT;
		strcpy(pack->nombre_tabla, spliteado[1]);
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		pack->key = atoi(spliteado[2]);
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->key);
	} else {
		printf("no entendi tu consulta\n");
	}
	string_iterate_lines(spliteado,free);
	free(spliteado);
}

void cargarPaqueteInsert(tInsert *pack, char* cons) {
	char** spliteado;
	char** value = string_split(cons, "\"");
	spliteado = string_n_split(cons, 4, " ");
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")
			&& strcmp(spliteado[3], "")) {
		pack->type = INSERT;
		pack->value = malloc(strlen(value[1])+1);
		pack->nombre_tabla = malloc(strlen(spliteado[1]) + 1);
		strcpy(pack->nombre_tabla,spliteado[1]);
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		pack->key = atoi(spliteado[2]);
		strcpy(pack->value, value[1]);
		pack->value_long = strlen(value[1]) + 1;
		pack->timestamp = 0;
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->key)
				+ sizeof(pack->value_long) + pack->value_long
				+ sizeof(pack->timestamp);
	} else {
		printf("no entendi tu consulta\n");
	}
	string_iterate_lines(spliteado,free);
	free(spliteado);
	string_iterate_lines(value,free);
	free(value);
}
void cargarPaqueteInsertLFS(tInsert *pack, char* cons) {
	char** spliteado;
	char** value = string_split(cons, "\"");
	spliteado = string_n_split(cons, 5, " "); //INTER T 3 "H" 23432?
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")
			&& strcmp(spliteado[3], "")) {
		pack->type = INSERT;
		pack->value = malloc(strlen(value[1])+1);
		pack->nombre_tabla = malloc(strlen(spliteado[1])+1);
		strcpy(pack->nombre_tabla,spliteado[1]);
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		pack->key = atoi(spliteado[2]);
		if(spliteado[4]==NULL)
			pack->timestamp=obtenerTimestamp();
		else
			pack->timestamp=atol(spliteado[4]);
		strcpy(pack->value, value[1]);
		pack->value_long = strlen(value[1]) + 1;
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->key)
				+ sizeof(pack->value_long) + pack->value_long + sizeof(pack->timestamp);
	} else {
		printf("no entendi tu consulta\n");
	}
	string_iterate_lines(spliteado,free);
	free(spliteado);
	string_iterate_lines(value,free);
	free(value);
}

void cargarPaqueteCreate(tCreate *pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 5, " ");
	if (strcmp(spliteado[1], "") && strcmp(spliteado[2], "")
			&& strcmp(spliteado[3], "") && strcmp(spliteado[4], "")) {
		pack->type = CREATE;
		pack->nombre_tabla = malloc(strlen(spliteado[1])+1);
		strcpy(pack->nombre_tabla, spliteado[1]);
		pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		pack->consistencia = malloc(strlen(spliteado[2])+1);
		strcpy(pack->consistencia,spliteado[2]);
		pack->consistencia_long = strlen(spliteado[2]) + 1;
		pack->particiones = atoi(spliteado[3]);
		pack->compaction_time = atoi(spliteado[4]);
		pack->length = sizeof(pack->type) + sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long + sizeof(pack->consistencia_long)
				+ pack->consistencia_long + sizeof(pack->particiones)
				+ sizeof(pack->compaction_time);
	} else {
		printf("no entendi tu consulta\n");
	}
	string_iterate_lines(spliteado,free);
	free(spliteado);
}

void cargarPaqueteDescribe(tDescribe *pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 2, " ");//DESCRIBE\n
		pack->type = DESCRIBE;
		if(spliteado[1] != NULL){
			pack->nombre_tabla = malloc(strlen(spliteado[1])+1);
			strcpy(pack->nombre_tabla, string_substring_until(spliteado[1], strlen(spliteado[1])));
			pack->nombre_tabla_long = strlen(spliteado[1]) + 1;
		}else{
			pack->nombre_tabla = malloc(strlen("")+1);
			strcpy(pack->nombre_tabla, "");
			pack->nombre_tabla_long = strlen("") + 1;
		}
		pack->length = sizeof(pack->type)
				+ sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long;
	string_iterate_lines(spliteado,free);
	free(spliteado);
}

void cargarPaqueteDrop(tDrop*pack, char* cons) {
	char** spliteado;
	spliteado = string_n_split(cons, 2, " ");
	pack->type = DROP;
	char** name_tabla = string_split(spliteado[1],"\n");
	pack->nombre_tabla = malloc(strlen(spliteado[1])+1);
	strcpy(pack->nombre_tabla,name_tabla[0]);
	strcat(pack->nombre_tabla,"\0");
	//strcat(name_tabla[0],name_tabla[1]);
	//strcpy(pack->nombre_tabla,);
	//strcpy(pack->nombre_tabla, spliteado[1]);
		pack->nombre_tabla_long = strlen(pack->nombre_tabla)+1;
		pack->length = sizeof(pack->type)
				+ sizeof(pack->nombre_tabla_long)
				+ pack->nombre_tabla_long;
	string_iterate_lines(spliteado,free);
	free(spliteado);
}

void cargarPaqueteJournal(tJournal* pack, char* cons){
	char** spliteado;
	spliteado = string_n_split(cons, 0, " ");
		pack->type = JOURNAL;
		pack->length = sizeof(pack->type);
		free(spliteado);
}

char* serializarSelect(tSelect* packageSelect) {
	packageSelect->length = sizeof(packageSelect->type)
			+ sizeof(packageSelect->nombre_tabla_long)
			+ packageSelect->nombre_tabla_long + sizeof(packageSelect->key);

	char *serializedPackage = malloc(packageSelect->length);
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(packageSelect->type);
	memcpy(serializedPackage + offset, &(packageSelect->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(packageSelect->nombre_tabla_long);
	memcpy(serializedPackage + offset, &(packageSelect->nombre_tabla_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageSelect->nombre_tabla_long;

	memcpy(serializedPackage + offset, (packageSelect->nombre_tabla),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint16_t);
	memcpy(serializedPackage + offset, &packageSelect->key, size_to_send);

	return serializedPackage;
}

int desSerializarSelect(tSelect* packageSelect, int socket) {

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));
	status = recv(socket, buffer, sizeof((packageSelect->nombre_tabla_long)),
			0); //recibo la longitud
	memcpy(&(packageSelect->nombre_tabla_long), buffer, buffer_size);

	if (!status)
		return 0;
	packageSelect->nombre_tabla = malloc(packageSelect->nombre_tabla_long);

	status = recv(socket, packageSelect->nombre_tabla,
			packageSelect->nombre_tabla_long, 0); //recibo el nombre de la tabla

	if (!status)
		return 0;
	//packageSelect->key = malloc(sizeof(uint16_t));
	status = recv(socket, &packageSelect->key, sizeof(packageSelect->key), 0); //recibo el nombre de la key
	if (!status)
		return 0;
	packageSelect->type = SELECT;
	free(buffer);

	return status;

}

char* serializarInsert(tInsert* packageInsert) {

	char* serializedPackage = malloc(packageInsert->length);
	int offset = 0;
	int size_to_send = 0;

	size_to_send = sizeof(packageInsert->type);
	memcpy(serializedPackage + offset, &(packageInsert->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(packageInsert->nombre_tabla_long);
	memcpy(serializedPackage + offset, &(packageInsert->nombre_tabla_long),	size_to_send);
	offset += size_to_send;

	size_to_send = packageInsert->nombre_tabla_long;
	memcpy(serializedPackage + offset, (packageInsert->nombre_tabla), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint16_t);
	memcpy(serializedPackage + offset, &packageInsert->key, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &packageInsert->value_long, size_to_send);
	offset += size_to_send;

	size_to_send = packageInsert->value_long;
	memcpy(serializedPackage + offset, (packageInsert->value), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(packageInsert->timestamp);
	memcpy(serializedPackage + offset, &packageInsert->timestamp, size_to_send);

	return serializedPackage;
}

int desSerializarInsert(tInsert* packageInsert, int socket) {

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	uint32_t nombrelong;
	status = recv(socket, buffer, sizeof((packageInsert->nombre_tabla_long)),
			0); //recibo la longitud
	memcpy(&(packageInsert->nombre_tabla_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageInsert->nombre_tabla = malloc(packageInsert->nombre_tabla_long);

	status = recv(socket, packageInsert->nombre_tabla,
			packageInsert->nombre_tabla_long, 0); //recibo el nombre de la tabla

	if (!status)
		return 0;
	//packageInsert->key = malloc(sizeof(uint16_t));
	status = recv(socket, &packageInsert->key, sizeof(packageInsert->key), 0); //recibo el key
	if (!status)
		return 0;
	uint32_t valuelong;
	status = recv(socket, buffer, sizeof((packageInsert->value_long)), 0); //recibo la longitud
	memcpy(&(packageInsert->value_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageInsert->value = malloc(packageInsert->value_long);

	status = recv(socket, packageInsert->value, packageInsert->value_long, 0); //recibo el value

	if (!status)
		return 0;
	status = recv(socket, &packageInsert->timestamp, sizeof(packageInsert->timestamp), 0);

	if (!status)
			return 0;
	free(buffer);
	packageInsert->length = sizeof(packageInsert->type)
			+ sizeof(packageInsert->nombre_tabla_long)
			+ packageInsert->nombre_tabla_long + sizeof(packageInsert->key)
			+ sizeof(packageInsert->value_long) + packageInsert->value_long
			+ sizeof(packageInsert->timestamp);
	packageInsert->type = INSERT;
	return status;

}

char* serializarCreate(tCreate* packageCreate) {

	char* serializedPackage = malloc(packageCreate->length);
	int offset = 0;
	int size_to_send = 0;

	size_to_send = sizeof(packageCreate->type);
	memcpy(serializedPackage + offset, &(packageCreate->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(packageCreate->nombre_tabla_long);
	memcpy(serializedPackage + offset, &(packageCreate->nombre_tabla_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageCreate->nombre_tabla_long;

	memcpy(serializedPackage + offset, (packageCreate->nombre_tabla),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(packageCreate->consistencia_long);
	memcpy(serializedPackage + offset, &(packageCreate->consistencia_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageCreate->consistencia_long;

	memcpy(serializedPackage + offset, (packageCreate->consistencia),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(int);
	memcpy(serializedPackage + offset, &packageCreate->particiones,
			size_to_send);

	offset += size_to_send;
	size_to_send = sizeof(int);
	memcpy(serializedPackage + offset, &packageCreate->compaction_time,
			size_to_send);
	offset += size_to_send;

	return serializedPackage;
}

int desSerializarCreate(tCreate* packageCreate, int socket) {

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	uint32_t nombrelong;
	status = recv(socket, buffer, sizeof((packageCreate->nombre_tabla_long)),
			0); //recibo la longitud
	memcpy(&(packageCreate->nombre_tabla_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageCreate->nombre_tabla = malloc(packageCreate->nombre_tabla_long);

	status = recv(socket, packageCreate->nombre_tabla,
			packageCreate->nombre_tabla_long, 0); //recibo el nombre de la tabla

	status = recv(socket, buffer, sizeof((packageCreate->consistencia_long)),
			0); //recibo la longitud
	memcpy(&(packageCreate->consistencia_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageCreate->consistencia = malloc(packageCreate->consistencia_long);

	status = recv(socket, packageCreate->consistencia,
			packageCreate->consistencia_long, 0); //recibo el nombre de la tabla

	if (!status)
		return 0;
	//packageCreate->particiones = malloc(sizeof(int));
	status = recv(socket, &packageCreate->particiones,
			sizeof(packageCreate->particiones), 0); //recibo particiones
	if (!status)
		return 0;

	//packageCreate->compaction_time = malloc(sizeof(int));
	status = recv(socket, &packageCreate->compaction_time,
			sizeof(packageCreate->compaction_time), 0); //recibo particiones
	if (!status)
		return 0;

	free(buffer);
	packageCreate->length = sizeof(packageCreate->type)
			+ sizeof(packageCreate->nombre_tabla_long)
			+ packageCreate->nombre_tabla_long
			+ sizeof(packageCreate->consistencia_long)
			+ packageCreate->consistencia_long
			+ sizeof(packageCreate->particiones)
			+ sizeof(packageCreate->compaction_time);
	packageCreate->type = CREATE;
	return status;

}

char* serializarDescribe(tDescribe* packageDescribe) {

	char* serializedPackage = malloc(packageDescribe->length);
	int offset = 0;
	int size_to_send = 0;

	size_to_send = sizeof(packageDescribe->type);
	memcpy(serializedPackage + offset, &(packageDescribe->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(packageDescribe->nombre_tabla_long);
	memcpy(serializedPackage + offset, &(packageDescribe->nombre_tabla_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageDescribe->nombre_tabla_long;

	memcpy(serializedPackage + offset, (packageDescribe->nombre_tabla),
			size_to_send);
	offset += size_to_send;
	return serializedPackage;
}

int desSerializarDescribe(tDescribe* packageDescribe, int socket) {
	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	status = recv(socket, buffer, sizeof((packageDescribe->nombre_tabla_long)), 0); //recibo la longitud
	memcpy(&(packageDescribe->nombre_tabla_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageDescribe->nombre_tabla = malloc(packageDescribe->nombre_tabla_long);

	status = recv(socket, packageDescribe->nombre_tabla, packageDescribe->nombre_tabla_long, 0); //recibo el nombre de la tabla
	free(buffer);
	packageDescribe->length = sizeof(packageDescribe->type)
			+ sizeof(packageDescribe->nombre_tabla_long)
			+ packageDescribe->nombre_tabla_long;
	packageDescribe->type = DESCRIBE;
	return status;
}

char* serializarDescribe_Response(t_describe *package) {

	char *serializedPackage;
	serializedPackage = malloc(sizeof(package->cant_tablas) + package->cant_tablas * sizeof(t_metadata));

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(package->cant_tablas);
//	printf("size to send %d \n",size_to_send);
	memcpy(serializedPackage + offset, &(package->cant_tablas), size_to_send);
	offset += size_to_send;

	for (int x = 0; x < package->cant_tablas; x++) {
		size_to_send = sizeof(t_metadata);
//		printf("size to send %d \n",size_to_send);
		memcpy(serializedPackage + offset, &(package->tablas[x]), size_to_send);
		offset += size_to_send;

	}

	return serializedPackage;
}

int desserializarDescribe_Response(t_describe* package, int socket) {

	int status = recv(socket, &(package->cant_tablas), sizeof(package->cant_tablas), MSG_WAITALL); //recibo el nombre de la key
	if (!status)
		return 0;
	if(status == -1)
		return -1;
	package->tablas = malloc(package->cant_tablas * sizeof(t_metadata));
	if (package->cant_tablas > 0) {
		for (int x = 0; x < package->cant_tablas; x++) {
			status = recv(socket, &package->tablas[x], sizeof(t_metadata), 0);
		}
	}

	return status;
}

char* serializarDrop(tDrop* packageDrop) {
	char* serializedPackage = malloc(packageDrop->length);
	int offset = 0;
	int size_to_send = 0;

	size_to_send = sizeof(packageDrop->type);
	memcpy(serializedPackage + offset, &(packageDrop->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(packageDrop->nombre_tabla_long);
	memcpy(serializedPackage + offset, &(packageDrop->nombre_tabla_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageDrop->nombre_tabla_long;

	memcpy(serializedPackage + offset, (packageDrop->nombre_tabla),
			size_to_send);
	offset += size_to_send;

	return serializedPackage;
}

int desSerializarDrop(tDrop* packageDrop, int socket) {
	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	status = recv(socket, buffer, sizeof((packageDrop->nombre_tabla_long)), 0); //recibo la longitud
	memcpy(&(packageDrop->nombre_tabla_long), buffer, buffer_size);
	if (!status)
		return 0;
	packageDrop->nombre_tabla = malloc(packageDrop->nombre_tabla_long);

	status = recv(socket, packageDrop->nombre_tabla,
			packageDrop->nombre_tabla_long, 0); //recibo el nombre de la tabla
	free(buffer);
	packageDrop->length = sizeof(packageDrop->type)
			+ sizeof(packageDrop->nombre_tabla_long)
			+ packageDrop->nombre_tabla_long;
	packageDrop->type = DROP;
	return status;
}

char* serializarJournal(tJournal* packageJournal){
	char* serializedPackage = malloc(packageJournal->length);
	int offset = 0;
	int size_to_send = 0;

	size_to_send = sizeof(packageJournal->type);
	memcpy(serializedPackage + offset, &(packageJournal->type), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	return serializedPackage;
}

int desSerializarJournal(tJournal* packageJournal, int socket){
	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));
	free(buffer);
	packageJournal->length = sizeof(packageJournal->type);
	packageJournal->type = JOURNAL;
	return status;
}
char* serializarRegistro(tRegistroRespuesta* reg) {
	reg->length = sizeof(reg->tipo) + sizeof(reg->value_long) + reg->value_long
			+ sizeof(unsigned long long) + sizeof(uint16_t);

	char *serializedPackage = malloc(reg->length);
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(reg->tipo);
	memcpy(serializedPackage + offset, &(reg->tipo), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(reg->key);
	memcpy(serializedPackage + offset, &(reg->key), size_to_send); //sizeof(int8_t)
	offset += size_to_send;

	size_to_send = sizeof(reg->value_long);
	memcpy(serializedPackage + offset, &(reg->value_long), size_to_send);
	offset += size_to_send;

	size_to_send = reg->value_long;

	memcpy(serializedPackage + offset, (reg->value), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(reg->timestamp);
	memcpy(serializedPackage + offset, &reg->timestamp, size_to_send);

	return serializedPackage;
}

int desSerializarRegistro(tRegistroRespuesta* reg, int socket) {

	int status;
	//int buffer_size;
	//char *buffer = malloc(buffer_size = sizeof(uint32_t));

	status = recv(socket, &reg->key, sizeof(reg->key), 0);
	if (!status)
		return 0;

	status = recv(socket, &reg->value_long, sizeof((reg->value_long)), 0);

	if (!status)
		return 0;
	reg->value = malloc(reg->value_long);

	status = recv(socket, reg->value, reg->value_long, 0);

	if (!status)
		return 0;

	status = recv(socket, &reg->timestamp, sizeof(reg->timestamp), 0);
	if (!status)
		return 0;

	//free(buffer);

	return status;

}

