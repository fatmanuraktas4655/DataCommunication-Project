#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT1 8000
#define PORT2 9000

void substitution(char *text) {
    if (strlen(text) == 0) return;
    int i = rand() % strlen(text);
    char old = text[i];
    text[i] = 'A' + rand() % 26;
    printf("LOG: Substitution (%c -> %c)\n", old, text[i]);
}

void swapping(char *text) {
    if (strlen(text) < 2) return;
    int i = rand() % (strlen(text) - 1);
    char tmp = text[i];
    text[i] = text[i+1];
    text[i+1] = tmp;
    printf("LOG: Swapping uygulandi\n");
}

int main() {
    WSADATA wsa;
    SOCKET listener, sender;
    struct sockaddr_in addr1, addr2;
    char packet[1024];

    srand(time(NULL));
    WSAStartup(MAKEWORD(2,2), &wsa);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT1);
    addr1.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr1, sizeof(addr1));
    listen(listener, 1);

    printf("Server: Client1 bekleniyor...\n");
    SOCKET c1 = accept(listener, NULL, NULL);
    recv(c1, packet, sizeof(packet), 0);

    char *msg = strtok(packet, "|");
    char *method = strtok(NULL, "|");
    char *control = strtok(NULL, "|");

    printf("Gelen: %s | %s\n", msg, method);

    /*if (rand() % 100 < 70)
        substitution(msg);
    else
        printf("LOG: Veri bozulmadi\n");*/
    
    int r = rand() % 3;

    if (r == 0) {
    substitution(msg);
    }
    else if(r==1){
    swapping(msg);
    }
    else{
        printf("LOG:Veri bozulmadı\n");
    }


    char new_packet[1024];
    sprintf(new_packet, "%s|%s|%s", msg, method, control);

    sender = socket(AF_INET, SOCK_STREAM, 0);
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(PORT2);
    addr2.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sender, (struct sockaddr*)&addr2, sizeof(addr2));
    send(sender, new_packet, strlen(new_packet), 0);

    printf("Server: Client2'ye iletildi\n");

    closesocket(c1);
    closesocket(listener);
    closesocket(sender);
    WSACleanup();
    return 0;
}
