#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <locale.h>
#include <curses.h>
#include <wchar.h>

int client_socket;
int isClosedSocket = 0;

// ncurses 윈도우
WINDOW *output_win;
WINDOW *input_win;

// 멀티바이트 문자의 실제 화면 너비를 계산

// 메시지를 출력
void print_message(const char *prefix, const char *message)
{
    wprintw(output_win, "%s %s\n", prefix, message); // 메시지 출력
    wrefresh(output_win);                            // 출력창 갱신
}
int calculate_width(const char *str)
{
    int width = 0;
    wchar_t wc;
    mbstate_t state;
    memset(&state, 0, sizeof(state));

    while (*str != '\0')
    {
        size_t len = mbrtowc(&wc, str, MB_CUR_MAX, &state);
        if (len == (size_t)-1 || len == (size_t)-2)
        {
            break; // 잘못된 멀티바이트 문자열
        }

        if (wcwidth(wc) > 0)
        {
            width += wcwidth(wc); // 멀티바이트 문자 너비
        }
        else
        {
            width += 1; // 기본 너비
        }

        str += len;
    }
    return width;
}
void refresh_input_prompt(const char *current_input)
{
    werase(input_win);                                    // 입력창 초기화
    box(input_win, 0, 0);                                 // 입력창 테두리 갱신
    mvwprintw(input_win, 1, 1, "메세지를 입력하시오 : "); // 프롬프트 출력
    mvwprintw(input_win, 1, 22, "%s", current_input);     // 현재 입력된 문자열 출력

    // 커서를 문자열의 마지막 정확한 위치로 이동
    int cursor_pos = 22 + calculate_width(current_input); // 멀티바이트 문자 너비 계산
    wmove(input_win, 1, cursor_pos);

    wrefresh(input_win); // 입력창 갱신
}

// 멀티바이트 문자의 실제 화면 너비를 계산

void handle_signal(int sig)
{
    endwin(); // ncurses 종료
    printf("\n클라이언트 종료 중...\n");
    close(client_socket);
    exit(0);
}

void *handle_send(void *arg)
{
    char client_message[256] = {0};
    int message_length = 0;

    while (1)
    {
        refresh_input_prompt(client_message); // 현재 입력 상태 갱신

        int ch = wgetch(input_win); // 입력 대기
        if (ch == '\n')             // Enter 입력
        {
            client_message[message_length] = '\0'; // 문자열 종료
            if (strcmp(client_message, "END") == 0)
            {
                isClosedSocket = 1;
                close(client_socket);
                pthread_exit(NULL);
            }

            send(client_socket, client_message, strlen(client_message), 0);
            print_message("[나]:", client_message); // 송신 메시지 출력

            memset(client_message, '\0', sizeof(client_message)); // 입력 초기화
            message_length = 0;                                   // 입력 길이 초기화
        }
        else if (ch == 127 || ch == KEY_BACKSPACE) // Backspace 처리
        {
            if (message_length > 0)
            {
                client_message[--message_length] = '\0'; // 마지막 문자 제거
            }
        }
        else if (ch != ERR && message_length < sizeof(client_message) - 1)
        {
            client_message[message_length++] = ch; // 입력된 문자 추가
        }

        refresh_input_prompt(client_message); // 갱신된 입력 상태 표시
    }
}

int main()
{
    // UTF-8 지원 설정
    setlocale(LC_ALL, "");

    signal(SIGINT, handle_signal);

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(TRUE);       // 커서 표시
    keypad(stdscr, TRUE); // 키패드 활성화
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // 출력창과 입력창 나누기
    output_win = newwin(rows - 3, cols, 0, 0);
    input_win = newwin(3, cols, rows - 3, 0);

    scrollok(output_win, TRUE); // 출력창 스크롤 가능
    box(input_win, 0, 0);       // 입력창 테두리

    mvwprintw(input_win, 1, 1, "메세지를 입력하시오 : ");
    wrefresh(input_win);

    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("socket 생성 실패");
        endwin();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8081);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect 실패");
        close(client_socket);
        endwin();
        return 1;
    }

    pthread_t thread;
    int ret = pthread_create(&thread, NULL, handle_send, NULL);
    if (ret != 0)
    {
        perror("쓰레드 생성 오류");
        close(client_socket);
        endwin();
        return 1;
    }
    pthread_detach(thread);

    char response_message[256];
    while (1)
    {
        memset(response_message, '\0', sizeof(response_message));
        int bytes_received = recv(client_socket, response_message, sizeof(response_message) - 1, 0);
        if (bytes_received < 0)
        {
            perror("recv 오류");
            close(client_socket);
            break;
        }
        else if (bytes_received == 0)
        {
            print_message("[서버]:", "연결이 종료되었습니다.");
            close(client_socket);
            break;
        }

        // 메시지 출력
        print_message("[상대]:", response_message);
    }

    endwin(); // ncurses 종료
    return 0;
}
