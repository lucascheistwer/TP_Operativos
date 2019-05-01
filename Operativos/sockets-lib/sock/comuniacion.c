#include "comunicacion.h"

int serializarRequest(t_Package_Request packageRequest, tPaquete* paqueteSerializado) {
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


int enviarPaquete(int clienteSocket, tPaquete* paquete_a_enviar) {

	return send(clienteSocket, paquete_a_enviar,(paquete_a_enviar)->length, 0);
}

int recibirPaquete(int socketReceptor, tPaquete* paquete_a_recibir) {
	int algo = recv(socketReceptor, &(paquete_a_recibir->type), sizeof(int8_t), MSG_WAITALL);
	if(algo != -1) printf("%d", paquete_a_recibir->type);
	int algo2 = recv(socketReceptor, &(paquete_a_recibir->length), sizeof(int16_t), MSG_WAITALL);
	int algo3 = recv(socketReceptor, &(paquete_a_recibir->payload), paquete_a_recibir->length, MSG_WAITALL);
	tCreate* createARecibir;
	tInsert* insertARecibir;
	tSelect* selectARecibir;
	tDescribe* describeARecibir;
	tDrop* dropARecibir;
	tJournal* journalARecibir;
	tAdd* addARecibir;
	switch (paquete_a_recibir->type) {

	case INSERT:

		//desSerializarInsert(paquete_a_recibir,insertARecibir);
		break;
	case SELECT:

		desSerializarSelect( paquete_a_recibir, selectARecibir);
		break;
	case CREATE:

		//desSserializarcreate(paquete_a_recibir,createARecibir);
		break;
	case DESCRIBE:

		//desSerializarDescribe(paquete_a_recibir,describeARecibir);
		break;
	case DROP:

		//desSserializarDescribe(paquete_a_recibir,dropARecibir);
		break;
	case JOURNAL:

	//	desSerializarJournal( paquete_a_recibir,journalARecibir);

		break;
	case ADD:

		//desSerializarAdd( paquete_a_recibir,addARecibir);
		break;

	}
	return algo;

}
int serializarSelect(tSelect packageSelect, tPaquete* paqueteSerializado) {


	int offset = 0;
	int size_to_send;

	paqueteSerializado->type=SELECT;
	//memcpy(paqueteSerializado->payload + offset, SELECT,sizeof(int8_t));//sizeof(int8_t)
	//offset += 1;

	size_to_send = strlen(packageSelect.nombre_tabla)+1;
	//(packageSelect.nombre_tabla)[strlen(packageSelect.nombre_tabla)]='\0';
	memcpy(paqueteSerializado->payload + offset,& (packageSelect.nombre_tabla),size_to_send);
	offset+=size_to_send;

	size_to_send = sizeof(int);
	memcpy(paqueteSerializado->payload + offset ,& (packageSelect.key),size_to_send);


	paqueteSerializado->length = offset + size_to_send+1;

	return 0;
}

int desSerializarSelect(tPaquete* paqueteSerializado, tSelect* packageSelect){
	packageSelect = malloc(sizeof(paqueteSerializado));
		int offset = 0;
		int size_to_send;
		for (size_to_send = 1;(paqueteSerializado->payload)[size_to_send - 1] != '\0';size_to_send++);

		memcpy(&(packageSelect->nombre_tabla), paqueteSerializado + offset, size_to_send);

		offset += size_to_send;
	    size_to_send = sizeof(int);

		memcpy(&(packageSelect->key), paqueteSerializado + offset, size_to_send);

		return 0;

}
