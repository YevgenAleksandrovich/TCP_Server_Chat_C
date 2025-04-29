// TCP_client_chat.c
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI receive_handler(LPVOID sock_ptr) {
    SOCKET sock = *(SOCKET *)sock_ptr;
    char buffer[1024];
    int size;

    while ((size = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[size] = '\0';
        printf("%s\n", buffer);
    }

    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[1024];

    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    CreateThread(NULL, 0, receive_handler, &sock, 0, NULL);

    while (1) {
        fgets(message, sizeof(message), stdin);
        send(sock, message, strlen(message), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
