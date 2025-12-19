#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


#define PORT 8000


/* --------- BINARY & PARITY --------- */
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

/* --------- 2D PARITY --------- */
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

/* --------- CRC32 --------- */
unsigned int crc32(const char *data) {
    unsigned int crc = 0xFFFFFFFF;

    while (*data) {
        crc ^= (unsigned char)(*data++);
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

/* --------- HAMMING --------- */
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

/* --------- IP CHECKSUM --------- */
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

/* --------- MAIN --------- */
int main() {

    WSADATA wsa;
if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
    printf("WSAStartup başarisiz\n");
    return 1;
}
   
    SOCKET sock;
    struct sockaddr_in server;
   
    char text[256], packet[1024], control[512];
    int choice;

    printf("gonderilecek metni girin: ");
    fgets(text, sizeof(text), stdin);
    text[strcspn(text, "\n")] = 0;

    printf("\n--- YONTEM SECİN ---\n");
    printf("1. Parity\n");
    printf("2. 2D Parity\n");
    printf("3. CRC\n");
    printf("4. Hamming\n");
    printf("5. Internet Checksum\n");
    printf("Secim (1-5): ");
    scanf("%d", &choice);

    char method[20];

    switch (choice) {
        case 1:
            strcpy(method, "PARITY");
            sprintf(control, "%c", get_parity(text));
            break;

        case 2:
            strcpy(method, "2DPARITY");
            get_2d_parity(text, control);
            break;

        case 3:
            strcpy(method, "CRC");
            sprintf(control, "%X", crc32(text));
            break;

        case 4:
            strcpy(method, "HAMMING");
            get_hamming(text, control);
            break;

        case 5:
            strcpy(method, "CHECKSUM");
            sprintf(control, "%X", ip_checksum(text));
            break;

        default:
            printf("Gecersiz secim!\n");
            return 0;
    }

    sprintf(packet, "%s|%s|%s", text, method, control);

   
   
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));
    send(sock, packet, strlen(packet), 0);

    printf("\nClient1: Paket gonderildi -> %s\n", packet);


    closesocket(sock);
    WSACleanup();
    return 0;

}




