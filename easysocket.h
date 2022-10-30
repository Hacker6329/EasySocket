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
    int tcp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis);
    int tcp_init_server(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog);
    int tcp_init_server_with_timeout(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog, int timeoutmillis);
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote);
    int tcp_receive(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_send(EasySocket* socketStructure, void* buffer, int bufferSize);
    void tcp_close(EasySocket* socketStructure);
    void tcp_close_and_cleanup(EasySocket* socketStructure);

    //Functions Pre-Declaration: UDP
	int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
    int udp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis);
	int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local);
    int udp_init_server_with_timeout(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local, int timeoutmillis);
	int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	void udp_close(EasySocket* socketStructure);
    void udp_close_and_cleanup(EasySocket* socketStructure);

    //Function Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout){
        return select(FD_SETSIZE, readSet, writeSet, exceptSet, timeout);
    }

    //Functions Declaration: TCP
    int tcp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

		//Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_addr.S_un.S_addr = inet_addr(serviceName);
        remote->sin_port = htons(servicePort);

        //Connecting to Server
        if (connect(socketStructure->socketFD, (struct sockaddr*)remote, sizeof(*remote)) == SOCKET_ERROR) {
            printf("Cannot connect to server: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

        return 0;
    }
    int tcp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis){
        
        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

        char buffer[32];
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

		//Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_addr.S_un.S_addr = inet_addr(serviceName);
        remote->sin_port = htons(servicePort);

        //Connecting to Server
        if (connect(socketStructure->socketFD, (struct sockaddr*)remote, sizeof(*remote)) == SOCKET_ERROR) {
            printf("Cannot connect to server: %d\n", WSAGetLastError());
            tcp_close(socketStructure);
            return WSAGetLastError();
        }

        return 0;
    }
    int tcp_init_server(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog){

        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if ((serverSocketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            tcp_close(serverSocketStructure);
            return WSAGetLastError();
        }

        //Preparing Server Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Binding Socket to Local Structure
        if (bind(serverSocketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            tcp_close(serverSocketStructure);
            return WSAGetLastError();
        }

        //Turning ServerSocket in Listening
        listen(serverSocketStructure->socketFD, backlog);

        return 0;
    }
    int tcp_init_server_with_timeout(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog, int timeoutmillis){

        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if ((serverSocketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            tcp_close(serverSocketStructure);
            return WSAGetLastError();
        }

        char buffer[32];
        setsockopt(serverSocketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));
        setsockopt(serverSocketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));

        //Preparing Server Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Binding Socket to Local Structure
        if (bind(serverSocketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            tcp_close(serverSocketStructure);
            return WSAGetLastError();
        }

        //Turning ServerSocket in Listening
        listen(serverSocketStructure->socketFD, backlog);

        return 0;
    }
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote){
        unsigned int remotelen = sizeof(*remote);
        int socketFD = accept(serverSocketStructure->socketFD, (struct sockaddr*)remote, &remotelen);
        if(socketFD == SOCKET_ERROR) {
            return SOCKET_ERROR;
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
    void tcp_close(EasySocket* socketStructure){
        closesocket(socketStructure->socketFD);
    }
    void tcp_close_and_cleanup(EasySocket* socketStructure){
        tcp_close(socketStructure);
        WSACleanup();
    }
    
    //Functions Declaration: UDP
    int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        //Preparing Local Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Could not bind socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        //Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_addr.S_un.S_addr = inet_addr(serviceName);
        remote->sin_port = htons(servicePort);

        return 0;
    }
    int udp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis){
        
        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup( MAKEWORD(2, 2), &data) != 0) {
            printf("WSA Startup Failed. Error Code: %d\n",WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            printf("Cannot create socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        char buffer[32];
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));

        //Preparing Local Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Could not bind socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        //Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_addr.S_un.S_addr = inet_addr(serviceName);
        remote->sin_port = htons(servicePort);

        return 0;
    }
    int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local){

        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup(MAKEWORD(2,2), &data) != 0) {
            printf("Failed. Error Code: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            printf("Could not create socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        //Preparing Server Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_addr.s_addr = ADDR_ANY;
        local->sin_port = htons(servicePort);

        //Bind Socket to Local Structure
        if(bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        return 0;
    }
    int udp_init_server_with_timeout(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local, int timeoutmillis){

        //Starting Up WSA
        struct WSAData data;
        if(WSAStartup(MAKEWORD(2,2), &data) != 0) {
            printf("Failed. Error Code: %d\n", WSAGetLastError());
            return WSAGetLastError();
        }

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            printf("Could not create socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }
        
        char buffer[32];
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, itoa(timeoutmillis,buffer,10), sizeof(timeoutmillis));

        //Preparing Server Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_addr.s_addr = ADDR_ANY;
        local->sin_port = htons(servicePort);

        //Bind Socket to Local Structure
        if(bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) == SOCKET_ERROR) {
            printf("Cannot bind socket: %d\n", WSAGetLastError());
            udp_close(socketStructure);
            return WSAGetLastError();
        }

        return 0;
    }
    int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        memset(buffer,0, bufferSize);
        int rcvlen = sizeof(*remote);
        int esito;
        if((esito = recvfrom(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, &rcvlen)) == SOCKET_ERROR) {
            return WSAGetLastError();
        }else{
            return esito;
        }
    }
    int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote){
        int esito;
        if((esito = sendto(socketStructure->socketFD, buffer, bufferSize, 0, (struct sockaddr*)remote, sizeof(*remote))) == SOCKET_ERROR) {
            return WSAGetLastError();
        }else{
            return esito;
        }
    }
    void udp_close(EasySocket* socketStructure){
        closesocket(socketStructure->socketFD);
    }
    void udp_close_and_cleanup(EasySocket* socketStructure){
        udp_close(socketStructure);
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
    int tcp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis);
    int tcp_init_server(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog);
    int tcp_init_server_with_timeout(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog, int timeoutmillis);
    int tcp_accept(EasySocket* serverSocketStructure, EasySocket* socketStructure, struct sockaddr_in* remote);
    int tcp_receive(EasySocket* socketStructure, void* buffer, int bufferSize);
    int tcp_send(EasySocket* socketStructure, void* buffer, int bufferSize);
    void tcp_close(EasySocket* socketStructure);
    void tcp_close_and_cleanup(EasySocket* socketStructure);

    //Functions Pre-Declaration: UDP
	int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote);
    int udp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis);
	int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local);
    int udp_init_server_with_timeout(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local, int timeoutmillis);
	int udp_receive(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	int udp_send(EasySocket* socketStructure, void* buffer, int bufferSize, struct sockaddr_in* remote);
	void udp_close(EasySocket* socketStructure);
    void udp_close_and_cleanup(EasySocket* socketStructure);

    //Function Declaration: Universal
    int easy_select(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet, struct timeval* timeout){
        return select(FD_SETSIZE, readSet, writeSet, exceptSet, timeout);
    }

    //Functions Declaration: TCP
    int tcp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        struct hostent *h;

        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror ("Creazione Socket: ");
            return -1;
        }

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Bind Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("errore Bind: ");
            tcp_close(socketStructure);
            return -2;
        }

        //Preparing Remote Structure
		memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_port = htons(servicePort);
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        //Connecting to Server
        if (connect (socketStructure->socketFD, (struct sockaddr*)remote, sizeof(*remote)) < 0) {
            perror("errore Sendro: ");
            tcp_close(socketStructure);
            return -3;
        }

        return 0;
    }
    int tcp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis){

        struct hostent *h;

        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror ("Creazione Socket: ");
            return -1;
        }

        struct timeval timeout;
        timeout.tv_sec = timeoutmillis/1000;
        timeoutmillis -= timeoutmillis/1000;
        timeout.tv_usec = timeoutmillis*1000;
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Bind Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror ("errore Bind: ");
            tcp_close(socketStructure);
            return -2;
        }

        //Preparing Remote Structure
		memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_port = htons(servicePort);
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        //Connecting to Server
        if (connect (socketStructure->socketFD, (struct sockaddr*)remote, sizeof(*remote)) < 0) {
            perror("errore Sendro: ");
            tcp_close(socketStructure);
            return -3;
        }

        return 0;
    }
    int tcp_init_server(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog){

        //Creating Socket
        if ((serverSocketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Creazione Socket: ");
            return -1;
        }

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Bind Socket to Local Structure
        if(bind(serverSocketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("errore Bind: ");
            tcp_close(serverSocketStructure);
            return -2;
        }

        //Turning ServerSocket in Listening
        listen(serverSocketStructure->socketFD, backlog);

        return 0;
    }
    int tcp_init_server_with_timeout(EasySocket* serverSocketStructure, int servicePort, struct sockaddr_in* local, int backlog, int timeoutmillis){

        //Creating Socket
        if ((serverSocketStructure->socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Creazione Socket: ");
            return -1;
        }

        struct timeval timeout;
        timeout.tv_sec = timeoutmillis/1000;
        timeoutmillis -= timeoutmillis/1000;
        timeout.tv_usec = timeoutmillis*1000;
        setsockopt(serverSocketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(serverSocketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        //Preparing Local Structure
		memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Bind Socket to Local Structure
        if(bind(serverSocketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("errore Bind: ");
            tcp_close(serverSocketStructure);
            return -2;
        }

        //Turning ServerSocket in Listening
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
    void tcp_close(EasySocket* socketStructure){
        close(socketStructure->socketFD);
    }
    void tcp_close_and_cleanup(EasySocket* socketStructure){
        tcp_close(socketStructure);
    }

    //Functions Declaration: UDP
    int udp_init_client(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote){
        
        struct hostent *h;

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation Error: ");
            return -1;
        }

        //Preparing Local Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;                
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("Binding Error: ");
            udp_close(socketStructure);
            return -2;
        }

        //Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_port = htons(servicePort);
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        return 0;
    }
    int udp_init_client_with_timeout(EasySocket* socketStructure, char* serviceName, int servicePort, struct sockaddr_in* local, struct sockaddr_in* remote, int timeoutmillis){

        struct hostent *h;

        //Creating Socket
        if((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation Error: ");
            return -1;
        }

        struct timeval timeout;
        timeout.tv_sec = timeoutmillis/1000;
        timeoutmillis -= timeoutmillis/1000;
        timeout.tv_usec = timeoutmillis*1000;
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        //Preparing Local Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;                
        local->sin_port = htons(INADDR_ANY);
        local->sin_addr.s_addr = htonl(INADDR_ANY);
        
        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("Binding Error: ");
            udp_close(socketStructure);
            return -2;
        }

        //Preparing Remote Structure
        memset(remote, 0, sizeof(*remote));
        remote->sin_family = AF_INET;
        remote->sin_port = htons(servicePort);
        h = gethostbyname(serviceName);
        remote->sin_addr.s_addr = *((int*) h->h_addr_list[0]);

        return 0;
    }
    int udp_init_server(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local){
        
        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation Error: ");
            return -1;
        }
        
        //Preparing Server Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("Binding Error: ");
            udp_close(socketStructure);
            return -2;
        }

        return 0;
    }
    int udp_init_server_with_timeout(EasySocket* socketStructure, int servicePort, struct sockaddr_in* local, int timeoutmillis){

        //Creating Socket
        if ((socketStructure->socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket Creation Error: ");
            return -1;
        }

        struct timeval timeout;
        timeout.tv_sec = timeoutmillis/1000;
        timeoutmillis -= timeoutmillis/1000;
        timeout.tv_usec = timeoutmillis*1000;
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(socketStructure->socketFD, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        //Preparing Server Structure
        memset(local, 0, sizeof(*local));
        local->sin_family = AF_INET;
        local->sin_port = htons(servicePort);
        local->sin_addr.s_addr = htonl(INADDR_ANY);

        //Binding Socket to Local Structure
        if (bind(socketStructure->socketFD, (struct sockaddr*)local, sizeof(*local)) < 0) {
            perror("Binding Error: ");
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
    void udp_close(EasySocket* socketStructure){
        close(socketStructure->socketFD);
    }
    void udp_close_and_cleanup(EasySocket* socketStructure){
        udp_close(socketStructure);
    }
    #endif
#endif
