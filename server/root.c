#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void ListenServer();
int main()
{
    printf("Hello, Server!\n");
    ListenServer();
    return 0;
}

void ListenServer()
{
    printf("Listen Server\n");

    struct sockaddr_in socketInfo, clientInfo;

    // 서버 소켓 설정
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_port = htons(8080);
    socketInfo.sin_addr.s_addr = INADDR_ANY;

    socklen_t sockLen = sizeof(socketInfo);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket 생성 실패");
        return;
    }

    // 소켓 바인딩
    if (bind(server_socket, (struct sockaddr *)&socketInfo, sockLen) < 0)
    {
        perror("bind 실패");
        close(server_socket);
        return;
    }

    // 클라이언트 접속 대기
    if (listen(server_socket, 5) < 0)
    {
        perror("listen 실패");
        close(server_socket);
        return;
    }

    printf("서버가 클라이언트 접속을 기다립니다...\n");

    socklen_t clientLen = sizeof(clientInfo);
    int client_socket = accept(server_socket, (struct sockaddr *)&clientInfo, &clientLen);
    if (client_socket < 0)
    {
        perror("accept 실패");
        close(server_socket);
        return;
    }

    printf("클라이언트 연결 성공: %s\n", inet_ntoa(clientInfo.sin_addr));

    // 클라이언트에게 메시지 전송
    char server_message[100] = "Hello, I am the server.";
    send(client_socket, server_message, strlen(server_message), 0);

    // 소켓 닫기
    close(client_socket);
    close(server_socket);
}
