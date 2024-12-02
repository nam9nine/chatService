#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    printf("hello client\n");
    int client_socket;
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

    while (1)
    {
        char client_message[100];
        memset(client_message, '\0', sizeof(client_message));
        printf("메세지를 입력하시오 : ");

        fgets(client_message, sizeof(client_message), stdin);
        client_message[strcspn(client_message, "\n")] = '\0'; // 입력받은 문자열 끝에 있는 개행 문자 제거

        if (strcmp(client_message, "END") == 0)
        {
            break;
        }
        send(client_socket, client_message, strlen(client_message), 0);
    }
    if (close(client_socket) < 0)
    {
        perror("client_socket 닫기 실패");
    }
    return 0;
}
