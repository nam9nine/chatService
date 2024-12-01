#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
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
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect 실패");
        return 1;
    }

    char server_response[100];
    recv(client_socket, &server_response, sizeof(server_response), 0);

    printf("서버 응답: %s\n", server_response);

    close(client_socket);
    return 0;
}
