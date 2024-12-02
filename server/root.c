#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

int server_socket;

void handle_signal(int sig)
{
    printf("\n서버 종료 중...\n");
    close(server_socket);
    exit(0);
}

void ListenServer()
{
    printf("Listen Server\n");

    struct sockaddr_in socketInfo, clientInfo;

    // 서버 소켓 설정
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_port = htons(8081);
    socketInfo.sin_addr.s_addr = INADDR_ANY;

    socklen_t sockLen = sizeof(socketInfo);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    while (1)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&clientInfo, &clientLen);
        if (client_socket < 0)
        {
            perror("accept 실패");
            close(server_socket);
            return;
        }

        printf("클라이언트 연결 성공: %s\n", inet_ntoa(clientInfo.sin_addr));
        printf("클라이언트 주소 : %s\n", inet_ntoa(clientInfo.sin_addr));
        // 클라이언트에게 메시지 전송

        while (1)
        {
            char request_message[100];
            memset(request_message, '\0', sizeof(request_message));
            int bytes_received = recv(client_socket, request_message, sizeof(request_message), 0);
            if (bytes_received <= 0)
            {
                if (bytes_received == 0)
                {
                    // 클라이언트가 정상적으로 연결을 종료
                    printf("클라이언트가 연결을 종료했습니다.\n");
                    if (close(client_socket) < 0)
                    {
                        perror("클라이언트 소켓 닫기 실패");
                    }
                    break;
                }
                else if (bytes_received < 0)
                {
                    // 오류 처리
                    perror("recv 오류");
                    if (close(client_socket) < 0)
                    {
                        perror("클라이언트 소켓 닫기 실패");
                    }
                    break;
                }
            }
            printf("클라이언트가 보낸 메세지 : %s\n", request_message);
        }
    }
    if (close(server_socket) < 0)
    {
        perror("서버 소켓 닫기 실패");
    }
}

int main()
{
    signal(SIGINT, handle_signal);
    printf("Hello, Server!\n");
    ListenServer();
    return 0;
}
