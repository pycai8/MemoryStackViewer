#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include <iostream>

using namespace std;

size_t randSize()
{
    FILE* fp = fopen("/dev/urandom", "rb");
    if (fp == 0) return 0;

    size_t c1 = (size_t)fgetc(fp);
    size_t c2 = (size_t)fgetc(fp);
    size_t ret = ((c1 << 8) | c2);
    //cout << "random: " << ret << endl;
    return ret;
}

unsigned long getCurrent()
{
    struct timeval tv = {0};
    int ret = gettimeofday(&tv, 0);
    if (ret == 0) return (unsigned long)tv.tv_sec * 1000000UL + (unsigned long)tv.tv_usec;

    int err = errno;
    cout << "gettimeofday fail:" << strerror(err) << endl;
    return 0;
}

void innerTest()
{
    {
        void* ptr = malloc(randSize()); 
        malloc_usable_size(ptr);
        free(ptr);
    }
    { 
        void* ptr1 = malloc(randSize());
        size_t newSize = randSize();
        void* ptr2 = realloc(ptr1, newSize);
        if (ptr2) 
        {
            malloc_usable_size(ptr2);
            free(ptr2);
        }
        else if (newSize > 0)
        {
            malloc_usable_size(ptr1);
            free(ptr1);
        }
    }
    {
        void* ptr = calloc(randSize(), randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }
    {
        void* ptr = valloc(randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }
    {
        void* ptr = pvalloc(randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }
    {
        void* ptr = memalign(1024, randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }
    /*{
        void* ptr = libc_memalign(1024, randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }*/
    {
        void* ptr = aligned_alloc(1024, randSize());
        malloc_usable_size(ptr);
        free(ptr);
    }
    {
        void* ptr = 0;
        int ret = posix_memalign(&ptr, 1024, randSize());
        if (ret == 0 && ptr != 0)
        {
            malloc_usable_size(ptr);
            free(ptr);
        }
    }
}

void singleTest();

void* threadEntry(void* arg)
{
    int count = (int)(unsigned long)arg;
    for (int i = 0; i < count; i++)
    {
        singleTest();
    }

    return 0;
}

/*
 * ./pft threadsCount testCount
 *  like:
 *       ./pft 10 10  =>   use 10 threads, every one run testCount of singleTest.
 * */
int main(int argc, char** argv)
{
    unsigned long startTime = getCurrent();

    if (argc != 3)
    {
        cout << "usage: " << argv[0] << " threadsCount testCount" << endl;
        return 0;
    }

    int threadsCount = atoi(argv[1]);
    int testCount = atoi(argv[2]);
    pthread_t* threads = (pthread_t*)malloc(threadsCount*sizeof(pthread_t));
    if (threads == 0)
    {
        cout << "malloc fail, no memory." << endl;
        return 0;
    }

    for (int i = 0; i < threadsCount; i++)
    {
        threads[i] = 0;
        int ret = pthread_create(&threads[i], 0, threadEntry, (void*)(unsigned long)testCount);
        if (ret != 0)
        {
            cout << "pthread_create fail, ret=" << ret << endl;
            threads[i] = 0;
        }
    }

    for (int i = 0; i < threadsCount; i++)
    {
        if (threads[i] == 0) continue;
        void* ret = 0;
        pthread_join(threads[i], &ret);
    }

    free(threads);

    unsigned long endTime = getCurrent();
    cout 
        << threadsCount 
        << " threads(" 
        << threadsCount*testCount 
        << " times) cost time: " 
        << endTime-startTime 
        << endl;
    return 0;
}

