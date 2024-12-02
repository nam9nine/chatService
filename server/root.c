#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

int server_socket;

void handle_signal(int sig)
{
    printf("\n서버 종료 중...\n");
    close(server_socket);
    exit(0);
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    printf("클라이언트 별도 쓰레드 할당\n");

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
                pthread_exit(NULL);
            }
            else if (bytes_received < 0)
            {
                // 오류 처리
                perror("recv 오류");
                if (close(client_socket) < 0)
                {
                    perror("클라이언트 소켓 닫기 실패");
                }
                pthread_exit(NULL);
            }
        }
        printf("클라이언트가 보낸 메세지 : %s\n", request_message);
    }
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

        printf("클라이언트 연결 성공\n");
        printf("클라이언트 주소 : %s\n", inet_ntoa(clientInfo.sin_addr));
        printf("클라이언트 포트 : %hu\n", htons(clientInfo.sin_port));

        // 여기에서 멀티 쓰레드 로직 작성
        // accept()에서 클라이언트 socket 받고, 쓰레드를 생성해서 별도로 recv(), send() 하기
        // 싱글 쓰레드로 recv를 무한정 기다리니까 다른 새 클라이언트가 서비스를 이용 못 함(근데 연결(3-way-handshake)은 됨)
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)&client_socket);

        // 다른 클라이언트를 기다리지 않고, 개별적으로 처리
        pthread_detach(thread);
        // 클라이언트 마다 쓰레드를 만들어주기는 하는데 이제 쓰레드가 종료 될 때 독립적으로 그 하나만 잘 종료될 수 있게 join을 하지 않고 distach를 한다.

        // if (close(server_socket) < 0)
        // {
        //     perror("서버 소켓 닫기 실패");
        // }
    }
}

int main()
{
    signal(SIGINT, handle_signal);
    printf("Hello, Server!\n");
    ListenServer();
    return 0;
}
