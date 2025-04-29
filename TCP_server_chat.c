// TCP_server_chat.c
#include <stdio.h>
#include <winsock2.h>
#include <process.h> // _beginthread
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 100

SOCKET client_sockets[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION cs;

void log_message(const char *message) {
    FILE *file = fopen("server_log.txt", "a");
    if (file) {
        fprintf(file, "%s\n", message);
        fclose(file);
    }
}

void broadcast_message(const char *msg, SOCKET sender) {
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender) {
            send(client_sockets[i], msg, strlen(msg), 0);
        }
    }
    LeaveCriticalSection(&cs);
}

void client_handler(void *socket_ptr) {
    SOCKET client_socket = *(SOCKET *)socket_ptr;
    char buffer[1024];
    int size;

    char *welcome = "Добро пожаловать в чат!\n";
    send(client_socket, welcome, strlen(welcome), 0);

    while ((size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[size] = '\0';
        printf("Получено: %s\n", buffer);
        log_message(buffer);
        broadcast_message(buffer, client_socket);
    }

    // Отключение клиента
    printf("Клиент отключился.\n");
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
            }
            client_count--;
            break;
        }
    }
    LeaveCriticalSection(&cs);
    closesocket(client_socket);
    _endthread();
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;

    InitializeCriticalSection(&cs);

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    listen(server_socket, 5);

    printf("Сервер запущен на порту 8888...\n");

    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        printf("Клиент подключен.\n");
        EnterCriticalSection(&cs);
        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count++] = client_socket;
            _beginthread(client_handler, 0, (void *)&client_socket);
        } else {
            printf("Слишком много клиентов!\n");
            closesocket(client_socket);
        }
        LeaveCriticalSection(&cs);
        Sleep(100); // for copy socket
    }

    DeleteCriticalSection(&cs);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
