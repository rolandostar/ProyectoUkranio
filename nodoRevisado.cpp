#include <iostream>
#include <string>
#include <vector>
#include "PaqueteDatagrama.h"
#include "SocketDatagrama.h"
#include "Searcher.h"

#include <cstring>
#include <stdio.h>

using namespace std;

struct operacion{
	int op;
	int v1;
	int v2;
	char arg[255];
};

int main(int argc, char const *argv[]){
	bool soynuevo = true;

	//int puerto = (argv[1] == NULL)?7777:atoi(argv[1]), b; // Puerto 7777 por default
	//cout << "Puerto: " << puerto << endl;
	SocketDatagrama s_send(0);
	SocketDatagrama s_recv(9444);

	s_send.activaBroadcast(true);

	struct operacion data_ping = {0,0,0,""};
	char broadcast[16] = "255.255.255.255";
	//char broadcast[16] = "127.0.0.1";
	PaqueteDatagrama p1((char*)&data_ping,sizeof(data_ping),broadcast,9444);
	cout << "Enviando ping inicial" << endl;
	s_send.envia(p1);
	while(1){	
		cout << "Esperando comando" << endl;
		PaqueteDatagrama p3(sizeof(struct operacion));
		s_recv.recibe(p3);
		if(!strcmp(p3.obtieneDireccion(),"192.168.43.120")) {
			cout << "Saltandome" << endl;
			continue;
		}
		struct operacion op_recv;
		memcpy(&op_recv, p3.obtieneDatos(), sizeof(struct operacion));

		switch(op_recv.op){
			case 0: { // Ping
				cout << "Recibi Ping ";
				struct operacion ping_reply = {0,1,0,""};
				PaqueteDatagrama p4((char*)&ping_reply,sizeof(struct operacion),p3.obtieneDireccion(),9444);
				if(op_recv.v1 == 0)	{
					cout << "Request - Enviando Reply" << endl;
					s_send.envia(p4);
					/*
					if(soynuevo){
						char  comando[120];
						sprintf(comando,"wget -R index.html -np -r http://%s:8000/ -P . -nH --cut-dirs=1 -P ./ARCHIVOS/",p3.obtieneDireccion());
					}
					*/
				}else{
					cout << "Reply" << endl;
				}
				break;
			}
			case 1: { // Busqueda
				cout << "Recibi Busqueda! OP:1" << endl;

				cout << op_recv.op << endl;
				cout << op_recv.v1 << endl;
				cout << op_recv.v2 << endl;
				cout << op_recv.arg << endl;

				createIndex();
				vector<pair<string,pair<int,int> > > found;
				string arg(op_recv.arg);
				found = searchInIndexFileByParts("INDEXFILE",arg,op_recv.v1,op_recv.v2);
				struct operacion encontrado;
				for(int i=0; i<found.size(); i++) {
					encontrado.op  = 2;
					encontrado.v1  = found[i].second.first;
					encontrado.v2  = found[i].second.second;
					memcpy(encontrado.arg,found[i].first.c_str(),found[i].first.size()+1);

					cout << encontrado.op << endl;
					cout << encontrado.v1 << endl;
					cout << encontrado.v2 << endl;
					cout << encontrado.arg << endl;

					PaqueteDatagrama p5((char*)&encontrado,sizeof(struct operacion),p3.obtieneDireccion(),9444);
					s_send.envia(p5);
				}
				encontrado.op  = 3;
				encontrado.v1  = 0;
				encontrado.v2  = 0;
				memcpy(encontrado.arg,"END",4);

				PaqueteDatagrama p6((char*)&encontrado,sizeof(encontrado),p3.obtieneDireccion(),p3.obtienePuerto());
				s_send.envia(p6);
				break;
			}
			case 4: {
				char comando[120];
				sprintf(comando,"curl http://%s:8000/text/%s --output ./ARCHIVOS/%s",p3.obtieneDireccion(),op_recv.arg,op_recv.arg);
				system(comando);
				break;
			}
		}
	}

	return 0;
}