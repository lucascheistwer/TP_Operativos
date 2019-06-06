#include "comunicacion.h"

int serializarRequest(t_Package_Request packageRequest,
		tPaquete* paqueteSerializado) {
	paqueteSerializado = malloc(sizeof(package));

	int offset = 0;
	int size_to_send;

	size_to_send = strlen(packageRequest.header) + 1;
	memcpy(paqueteSerializado->payload + offset, packageRequest.header,
			size_to_send);
	offset += size_to_send;

	size_to_send = strlen(packageRequest.query) + 1;
	memcpy(paqueteSerializado->payload + offset, packageRequest.query,
			size_to_send);
	paqueteSerializado->length = offset + size_to_send;

	return paqueteSerializado;
}

int enviarPaquete(int clienteSocket, char* payload, uint32_t size) {

	return send(clienteSocket, payload, size, 0);
}

type leerHeader(int socket) {
	type header;
	recv(socket, &header, sizeof(type), MSG_WAITALL);
	return header;
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

	uint32_t nombrelong;
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
	packageSelect->key = malloc(sizeof(uint16_t));
	status = recv(socket, &packageSelect->key, sizeof(packageSelect->key), 0); //recibo el nombre de la key
	if (!status)
		return 0;

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
	memcpy(serializedPackage + offset, &(packageInsert->nombre_tabla_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = packageInsert->nombre_tabla_long;

	memcpy(serializedPackage + offset, (packageInsert->nombre_tabla),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint16_t);
	memcpy(serializedPackage + offset, &packageInsert->key, size_to_send);

	offset += size_to_send;
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &packageInsert->value_long,
			size_to_send);
	offset += size_to_send;

	size_to_send = packageInsert->value_long;
	memcpy(serializedPackage + offset, (packageInsert->value), size_to_send);
	offset += size_to_send;

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
	packageInsert->key = malloc(sizeof(uint16_t));
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

	free(buffer);
	packageInsert->length = sizeof(packageInsert->type)
			+ sizeof(packageInsert->nombre_tabla_long)
			+ packageInsert->nombre_tabla_long + sizeof(packageInsert->key)
			+ sizeof(packageInsert->value_long) + packageInsert->value_long;

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
	packageCreate->particiones = malloc(sizeof(int));
		status = recv(socket, &packageCreate->particiones, sizeof(packageCreate->particiones), 0); //recibo particiones
		if (!status)
			return 0;

		packageCreate->compaction_time = malloc(sizeof(int));
				status = recv(socket, &packageCreate->compaction_time, sizeof(packageCreate->compaction_time), 0); //recibo particiones
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

	return status;

}

char* serializarRegistro(tRegistroRespuesta* reg){
		reg->length = sizeof(reg->tipo) + sizeof(reg->value_long) + reg->value_long + sizeof(int) + sizeof(uint16_t);

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
		memcpy(serializedPackage + offset, &(reg->value_long),
				size_to_send);
		offset += size_to_send;

		size_to_send = reg->value_long;

		memcpy(serializedPackage + offset, (reg->value),
				size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(int);
		memcpy(serializedPackage + offset, &reg->timestamp, size_to_send);

		return serializedPackage;
}


int desSerializarRegistro(tRegistroRespuesta* reg, int socket) {

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	reg->key = malloc(sizeof(uint16_t));
		status = recv(socket, &reg->key, sizeof(reg->key), 0); //recibo el nombre de la key
		if (!status)
			return 0;

	uint32_t nombrelong;
	status = recv(socket, buffer, sizeof((reg->value_long)),
			0); //recibo la longitud
	memcpy(&(reg->value_long), buffer, buffer_size);

	if (!status)
		return 0;
	reg->value = malloc(reg->value_long);

	status = recv(socket, reg->value,reg->value_long, 0); //recibo el nombre de la tabla

	if (!status)
		return 0;

	reg->timestamp = malloc(sizeof(int));
			status = recv(socket, &reg->timestamp, sizeof(reg->timestamp), 0); //recibo el nombre de la key
			if (!status)
				return 0;

	free(buffer);

	return status;

}

