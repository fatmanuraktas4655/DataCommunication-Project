#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 9000

/* ------------------ ORTAK FONKSİYONLAR ------------------ */

void text_to_binary(const char *text, char *binary) {
    binary[0] = '\0';
    for (int i = 0; text[i]; i++) {
        char temp[9];
        sprintf(temp, "%08b", text[i]);
        strcat(binary, temp);
    }
}

char get_parity(const char *text) {
    char binary[4096];
    int ones = 0;

    text_to_binary(text, binary);
    for (int i = 0; binary[i]; i++)
        if (binary[i] == '1') ones++;

    return (ones % 2 == 0) ? '0' : '1';
}

void get_2d_parity(const char *text, char *result) {
    int col[8] = {0};
    char rows[256] = "";

    for (int i = 0; text[i]; i++) {
        char bin[9];
        int ones = 0;
        sprintf(bin, "%08b", text[i]);

        for (int j = 0; j < 8; j++) {
            if (bin[j] == '1') {
                ones++;
                col[j]++;
            }
        }
        strcat(rows, (ones % 2 == 0) ? "0" : "1");
    }

    strcpy(result, rows);
    for (int i = 0; i < 8; i++)
        strcat(result, (col[i] % 2 == 0) ? "0" : "1");
}

unsigned int crc32(const char *data) {
    unsigned int crc = 0xFFFFFFFF;

    while (*data) {
        crc ^= (unsigned char)(*data++);
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

void get_hamming(const char *text, char *result) {
    result[0] = '\0';

    for (int i = 0; text[i]; i++) {
        char b[9];
        int d[8];
        sprintf(b, "%08b", text[i]);

        for (int j = 0; j < 8; j++)
            d[j] = b[j] - '0';

        int p1 = d[0]^d[1]^d[3]^d[4]^d[6];
        int p2 = d[0]^d[2]^d[3]^d[5]^d[6];
        int p4 = d[1]^d[2]^d[3]^d[7];
        int p8 = d[4]^d[5]^d[6]^d[7];

        char temp[5];
        sprintf(temp, "%d%d%d%d", p1, p2, p4, p8);
        strcat(result, temp);
    }
}

unsigned short ip_checksum(const char *text) {
    unsigned int sum = 0;
    int len = strlen(text);

    for (int i = 0; i < len; i += 2) {
        unsigned short word = text[i] << 8;
        if (i + 1 < len)
            word |= text[i + 1];
        sum += word;
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;
}

/* ------------------ CLIENT 2 ------------------ */

int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);

    char buffer[1024];

    WSAStartup(MAKEWORD(2,2), &wsa);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server, sizeof(server));
    listen(server_sock, 1);

    printf("Client 2: Dinleniyor (Port %d)...\n", PORT);

    client_sock = accept(server_sock, (struct sockaddr*)&client, &client_len);
    recv(client_sock, buffer, sizeof(buffer), 0);

    char *message = strtok(buffer, "|");
    char *method  = strtok(NULL, "|");
    char *incoming = strtok(NULL, "|");

    printf("\n==============================\n");
    printf("Gelen mesaj     : %s\n", message);
    printf("Method : %s\n", method);
    printf("Gelen kontrol kodu: %s\n", incoming);

    char calculated[512] = "";

    if (strcmp(method, "PARITY") == 0) {
        sprintf(calculated, "%c", get_parity(message));
    }
    else if (strcmp(method, "2DPARITY") == 0) {
        get_2d_parity(message, calculated);
    }
    else if (strcmp(method, "CRC") == 0) {
        sprintf(calculated, "%X", crc32(message));
    }
    else if (strcmp(method, "HAMMING") == 0) {
        get_hamming(message, calculated);
    }
    else if (strcmp(method, "CHECKSUM") == 0) {
        sprintf(calculated, "%X", ip_checksum(message));
    }
    else {
        printf("ERROR: Bilinmeyen yontem!\n");
    }

    printf("Hesaplanan Kod    : %s\n", calculated);

    if (strcmp(calculated, incoming) == 0)
        printf(">>> STATUS: DATA CORRECT\n");
    else
        printf(">>> STATUS: DATA CORRUPTED\n");

    printf("==============================\n");

    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
