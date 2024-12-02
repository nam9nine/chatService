#include <stdio.h>
#include <pthread.h>

int counter = 0; // 공유 자원
pthread_mutex_t mutex;

void *thread_function(void *args)
{
    for (int i = 0; i < 10; i++)
    {
        // mutex 객체를 가진 쓰레드가 다음 코드를 실행
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main()
{
    printf("heelo");

    pthread_t threads[2];

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < 2; i++)
    {
        pthread_create(&threads[i], NULL, thread_function, NULL);
    }

    for (int i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("모든 쓰레드 작업 끝\n");
    printf("최종 coutner 값 : %d\n", counter);

    pthread_mutex_destroy(&mutex);
}