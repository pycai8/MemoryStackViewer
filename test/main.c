#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

struct TEST
{
    int a;
    int b;
    char data[100];
};

void main()
{
    printf("test: start\n");
    struct TEST* test = (struct TEST*)malloc(sizeof(struct TEST));
    if (!test)
    {
        printf("test: malloc fail.");
        return;
    }

    test->a = 100;
    test->b = 150;
    memset(test->data, 0, sizeof(test->data));
    strcat(test->data, "hello world!");
    printf("test: a=%d, b=%d, data=%s\n", test->a, test->b, test->data);

    free(test);
    test = 0;
    printf("test: end\n");
}
