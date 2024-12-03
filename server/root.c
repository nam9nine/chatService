#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

int server_socket;
int client_socket_id[2]; // 2개의 유저만 테스트
int isStart = 0;
int test_index = -1; // 추후에 유저 아이디로 변환

pthread_mutex_t mutex;
struct Info
{
    int index;
    int client_socket;
};

void handle_signal(int sig)
{
    printf("\n서버 종료 중...\n");
    close(server_socket);
    exit(0);
}

void handle_recv_send_err(int flag, int client_socket)
{
    if (flag <= 0)
    {
        if (flag == 0)
        {
            // 클라이언트가 정상적으로 연결을 종료한 경우
            printf("클라이언트가 연결을 정상 종료했습니다.\n");
        }
        else if (errno == ECONNRESET)
        {
            // 클라이언트가 강제 종료된 경우
            printf("클라이언트가 강제 종료되었습니다.\n");
        }
        else
        {
            // 기타 오류
            perror("recv/send 오류");
        }

        // 공통적으로 소켓 정리
        close(client_socket);
        pthread_exit(NULL);
    }
}

void *handle_client(void *arg)
{

    struct Info *info = (struct Info *)arg;
    int client_socket = info->client_socket;
    client_socket_id[info->index] = client_socket;
    int peer_client_socket;

    while (!isStart)
    {
        printf("다른 클라이언트 찾는 중\n");
        sleep(2);
    }

    for (int i = 0; i < 2; i++)
    {
        if (client_socket_id[i] != client_socket)
        {
            peer_client_socket = client_socket_id[i];
            break;
        }
    }

    while (1)
    {
        char request_message[100];
        memset(request_message, '\0', sizeof(request_message));

        int bytes_received = recv(client_socket, request_message, sizeof(request_message), 0);
        handle_recv_send_err(bytes_received, client_socket);
        printf("클라이언트가 보낸 메세지 : %s\n", request_message);

        send(client_socket, request_message, strlen(request_message), 0);
        send(peer_client_socket, request_message, strlen(request_message), 0);
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
        int ret;
        // 테스트용 유저 정보 생성
        test_index++;

        struct Info testInfo;
        testInfo.client_socket = client_socket;
        printf("client socket : %d\n", client_socket);

        testInfo.index = test_index;

        if (test_index == 1)
        {
            isStart = 1;
        }
        ret = pthread_create(&thread, NULL, handle_client, (void *)&testInfo);
        if (ret != 0)
        {
            perror("쓰레드 생성 오류");
            close(client_socket); // 서버에서 연결 종료 -> timeout -> 보안 문제
        }

        // 다른 클라이언트를 기다리지 않고, 개별적으로 처리
        pthread_detach(thread);
        // 클라이언트 마다 쓰레드를 만들어주기는 하는데 이제 쓰레드가 종료 될 때 독립적으로 그 하나만 잘 종료될 수 있게 join을 하지 않고 detach 한다.
    }
}

int main()
{
    signal(SIGINT, handle_signal);
    printf("Hello, Server!\n");
    memset(client_socket_id, 0, sizeof(client_socket_id) / sizeof(int));
    ListenServer();
    return 0;
}
