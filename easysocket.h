#ifndef EASY_SOCKET_H
#define EASY_SOCKET_H

//Including Libraries
#include <stdio.h>
#include <string.h>
#define LOCAL_ADDRESS "127.0.0.1"
#define USE_DEPRECATED 0

#ifdef _WIN32

    //Needed Libraries
    #include <WinSock2.h>
    #pragma comment(lib, "ws2_32") //Funziona Solo VS, per compilazione manuale usa "-lwsock32"

    //EasySocket Windows Structure
    typedef struct{
		SOCKET socketFD;
	}EasySocket;

    //Function Pre-Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout);

    //Functions Pre-Declaration: TCP
    int tcp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
    int tcp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local);
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote);
    int tcp_receive(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_send(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_close(EasySocket* socketStructure);

    //Functions Pre-Declaration: UDP
	int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
	int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local);
	int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_close(EasySocket* socketStructure);

    //Function Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout){
        return select(FD_SETSIZE, readSet, writeSet, exceptSet, timeout);
    }

    //Functions Declaration: TCP
    //TODO
    
    //Functions Declaration: UDP
    int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0){
            printf("Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
            printf("Could not create socket: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        //Prepare Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_addr.S_un.S_addr = inet_addr(serviceName);
        remote->sin_port = htons(servicePort);

        return 0;
    }
    int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local){

        struct WSAData data;
        if(WSAStartup(MAKEWORD(2,2), &data) != 0){
            printf("Failed. Error Code: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
            printf("Could not create socket: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        //Prepare Server Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_addr.s_addr = ADDR_ANY;
        local->sin_port = htons(servicePort);

        if(bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR){
            printf("Bind failed with error code: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        return 0;
    }
    int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        memset(buffer,0, bufferSize);
        int rcvlen = sizeof(*remote);
        int esito;
        if((esito = recvfrom(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, &rcvlen)) == SOCKET_ERROR){
            return WSAGetLastError();
        }else{
            return esito;
        }
    }
    int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        int esito;
        if((esito = sendto(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, sizeof(*remote))) == SOCKET_ERROR){
            return WSAGetLastError();
        }else{
            return esito;
        }
    }
    int udp_close(EasySocket* socketStructure){
        closesocket(socketStructure->socketFD);
        WSACleanup();
    }

#else

    //Needed Libraries
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <netdb.h>
    #include <sys/time.h>
    #include <sys/select.h>

    //EasySocket UNIX Structure
    typedef struct{
        int socketFD;
    }EasySocket;

    //Function Pre-Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout);

    //Functions Pre-Declaration: TCP
    int tcp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
    int tcp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local, int backlog);
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote);
    int tcp_receive(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_send(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_close(EasySocket* socketStructure);

    //Functions Pre-Declaration: UDP
	int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
	int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local);
	int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_close(EasySocket* socketStructure);

    //Function Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout){
        return select(FD_SETSIZE, readSet, writeSet, exceptSet, timeout);
    }

    //Functions Declaration: TCP
    int tcp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        struct hostent *h;

        if ((socketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror ("Creazione Socket: ");
            return -1;
        }

		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind (socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("errore Bind: ");
            close(socketStructure->socketFD);
            return -2;
        }

		memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_port = htons(servicePort);
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        if (connect (socketStructure->socketFD, (struct sockaddr*)remote, sizeof(*remote)) < 0) {
            perror ("errore Sendro: ");
            close(socketStructure->socketFD);
            return -3;
        }

        return 0;
    }
    int tcp_init_server(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog){

        if ((serverSocketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror ("Creazione Socket: ");
            return -1;
        }

		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind (serverSocketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("errore Bind: ");
            close(serverSocketStructure->socketFD);
            return -2;
        }

        listen(serverSocketStructure->socketFD, backlog);

        return 0;
    }
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote){
        unsigned int remotelen = sizeof(*remote);
        int socketFD = accept(serverSocketStructure->socketFD, (struct sockaddr*)remote, &remotelen);
        if(socketFD == -1){
            return -1;
        }
        socketStructure->socketFD = socketFD;
        return 0;
    }
    int tcp_receive(EasySocket* socketStructure, void* buffer, int bufferSize){
        return recv(socketStructure->socketFD, buffer, bufferSize, 0);
    }
    int tcp_send(EasySocket* socketStructure, void* buffer, int bufferSize){
        return send(socketStructure->socketFD, buffer, bufferSize, 0);
    }
    int tcp_close(EasySocket* socketStructure){
        return close(socketStructure->socketFD);
    }

    //Functions Declaration: UDP
    int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        struct hostent *h;

        // Crea il socket della famiglia IPv4 di tipo Datagram (UDP)
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // AF_INET  Address Family IPv4   
            perror ("Socket Creation Error: ");
            return -1;
        }

        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;                
        local->sin_port = htons(INADDR_ANY);         // Il sistema sceglie la porta
        local->sin_addr.s_addr = htonl(INADDR_ANY);  // Utilizza qualunque interfaccia
        
        // Configura il socket con l'indirizzo locale
        if (bind (socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("Binding Error: ");
            udp_close(socketStructure);
            return -2;
        }

        // Compila l'indirizzo remoto
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET; // Address Family IPv4
        remote->sin_port = htons(servicePort); // Chiamerò la porta del servizio
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        return 0;
    }
    int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local){
        
        // Crea il socket della famiglia IPv4 di tipo Datagram (UDP)
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror ("Socket Creation Error: ");
            return -1;
        }
        
        // compila indirizzo del server
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind (socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("Binding Error: ");
            udp_close(socketStructure);
            return -2;
        }

        return 0;
        
    }
    int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        unsigned int rcvlen = sizeof(*remote);
        return recvfrom(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, &rcvlen);
    }
    int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        return sendto(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, sizeof(*remote));
    }
    int udp_close(EasySocket* socketStructure){
        close(socketStructure->socketFD);
    }
    #endif
#endif
