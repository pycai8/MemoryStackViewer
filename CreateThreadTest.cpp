#include <iostream>
#include <pthread.h>
#include <string>
#include <string.h>
#include <sys/unistd.h>

static void* threadEntry(void* arg);
static void createThread();
static int g_count = 0;
static char* g_str = 0;

static void doSomeThing()
{
    if (g_str != 0)
    {
        std::cout << g_count << "," << g_str << std::endl;
        delete[] g_str;
        g_str = 0;
    }
    else
    {
        char* tmp = new char[100];
        memset(tmp, 0, 100 * sizeof(char));
        strcpy(tmp, "pycai, hold on!!!");
        g_str = tmp;
    }
}

static void* threadEntry(void* arg)
{
    __sync_add_and_fetch(&g_count, 1);
    
    int count = 0;
    while (++count <= 10)
    {
        sleep(2);
        doSomeThing();
        createThread();
    }

    __sync_sub_and_fetch(&g_count, 1);
    return 0;
}

static void createThread()
{
    pthread_t th = 0;
    pthread_attr_t attr = {0};
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, threadEntry, 0);
    pthread_attr_destroy(&attr);
}

int main(int argc, char** argv)
{
    createThread();

    while(true)
    {
        sleep(2);
    }

    return 0;
}

