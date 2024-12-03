#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

int client_socket;
int isClosedSocket = 0;

void handle_signal(int sig)
{
    printf("클라이언트 종료 중...\n");
    close(client_socket);
    exit(0);
}

void *handle_send(void *arg)
{
    while (1)
    {
        char client_message[100];
        memset(client_message, '\0', sizeof(client_message));
        printf("메세지를 입력하시오 : ");

        fgets(client_message, sizeof(client_message), stdin);
        client_message[strcspn(client_message, "\n")] = '\0'; // 입력받은 문자열 끝에 있는 개행 문자 제거

        if (strcmp(client_message, "END") == 0)
        {
            isClosedSocket = 1;
            close(client_socket);
            pthread_exit(NULL);
        }
        send(client_socket, client_message, strlen(client_message), 0);
    }
}

int main()
{
    signal(SIGINT, handle_signal);
    printf("hello client\n");
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("socket 생성 실패");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8081);
    server_address.sin_addr.s_addr = inet_addr("192.168.1.100");

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect 실패");
        close(client_socket);
        return 1;
    }
    // 실시간으로 상대의 메세지를 응답 받기 위해서 send()를 별도의 쓰레드로 처리

    pthread_t thread;
    int ret;
    ret = pthread_create(&thread, NULL, handle_send, NULL);
    if (ret != 0)
    {
        perror("쓰레드 생성 오류");
        close(client_socket); // 서버에서 연결 종료 -> timeout -> 보안 문제
    }
    pthread_detach(thread);

    // 클라이언트 소켓 열려있을 때만
    while (client_socket > 0)
    {
        char response_message[100];
        memset(response_message, '\0', sizeof(response_message));
        int bytes_received = recv(client_socket, response_message, sizeof(response_message), 0);
        if (bytes_received < 0)
        {
            perror("recv 오류");
            close(client_socket);
            break;
        }
        else if (bytes_received == 0)
        {
            printf("서버가 연결을 종료했습니다.\n");
            close(client_socket);
            break;
        }
        printf("\n상대가 보낸 메세지 : %s\n", response_message);
    }

    return 0;
}
