/*
 * modify addr2line.c from binutils-2.37.tar.gz like below
 *
 * 1 =>
 *    #include "trb.h"
      static bool unwind_inlines;
 * 2=>
 *    if (file_name == NULL)
          file_name = argv[2];
 *
 * */

#ifndef __TRANSLATE_BACKTRACE_H__
#define __TRANSLATE_BACKTRACE_H__

#include <errno.h>

typedef struct
{
    unsigned long total;
    unsigned long count;
    int len[32];
    char bts[32][1024];
} StackInfo;

static char my_ret[10240] = {0};
static int my_len = 0;

int myMain(int argc, char** argv);

static void trOne(char* fileAddr, int len, char* outBuf, int bufLen)
{
    char* file = fileAddr;
    char* diff = 0;
    int i = 0;
    int zeroPos = 0;
    for (i = 0; i < len; i++)
    {
        if (fileAddr[i] == ' ' && i+1 < len && fileAddr[i+1] != ' ')
        {
            zeroPos = i;
            fileAddr[i] = 0;
            diff = file + i + 1;
            break;
        }
    }

    my_len = 0;
    int argc = 4;
    char* argv[4] = {"./trb", "-Cfpie", file, diff};
    int ret = myMain(argc, argv);
    fileAddr[zeroPos] = ' ';
    if (ret != 0) return;
    snprintf(outBuf, bufLen, "%s", my_ret);
    int tmpLen = strlen(outBuf);
    if (tmpLen > 0 && outBuf[tmpLen - 1] == '\n') outBuf[tmpLen - 1] = 0;
}

static int readLine(FILE* fp, char* line, int len)
{
    char* ret = fgets(line, len, fp);
    if (ret == 0) return 0;

    len = strlen(line);
    if (len > 0 && line[len-1] == '\n')
    {
        line[len-1] = 0;
        len--;
    }

    return len;
}

static void getTotalCount(char* line, int len, unsigned long* total, unsigned long* count)
{
    int flag = 0;
    int i = 0;
    *total = 0;
    *count = 0;
    for (i = 0; i < len; i++)
    {
        if (line[i] == ',')
        {
            flag = 1;
            continue;
        }

        if (!('0' <= line[i] && line[i] <= '9'))
        {
            continue;
        }

        unsigned long* tmp = (flag ? total : count);
        *tmp *= 10;
        *tmp += line[i] - '0';
    }
}

static void trBacktraces(FILE* fpBt, FILE* fpTxt)
{
    // deal head
    char line[1024] = {0};
    int len = 0;
    while ((len=readLine(fpBt, line, sizeof(line))) > 0)
    {
        fprintf(fpTxt, "%s\n", line);
    }
    // skip empty line
    len = readLine(fpBt, line, sizeof(line));

    // deal body
    int count = 1000;
    int size = 0;
    StackInfo** infos = (StackInfo**)malloc(count * sizeof(StackInfo*)); // fixme malloc fail
    while ((len=readLine(fpBt, line, sizeof(line))) > 0)
    {    
        StackInfo* psi = (StackInfo*)malloc(sizeof(StackInfo)); // fixme malloc fail
        getTotalCount(line, len, &psi->total, &psi->count);
        int idx = 0;
        while (idx < 32 && (psi->len[idx]=readLine(fpBt, psi->bts[idx], sizeof(psi->bts[idx]))) > 0)
        {
            idx++;
        }
           
        if (size+1 > count)
        {
            count += 1000;
            infos = (StackInfo**)realloc(infos, count * sizeof(StackInfo*)); // fixme realloc fail
        }
        infos[size] = psi;
        size++;
    }

    // sort infos
    int i = 0;
    for (i = 0; size > 0 && i < size-1; i++)
    {
        int j = 0;
        int max = i;
        for (j = i+1; j < size; j++)
        {
            if (infos[j]->total > infos[max]->total) max = j;
        }
        // swap max and i
        StackInfo* tmp = infos[i];
        infos[i] = infos[max];
        infos[max] = tmp;
    }

    // print body
    fprintf(fpTxt, "\n\n");
    for (i = 0; i < size; i++)
    {
        fprintf(fpTxt, "total:%lu, count: %lu\n", infos[i]->total, infos[i]->count);
        int j = 0;
        for (j = 0; j < 32; j++)
        {
            if (infos[i]->len[j] == 0) break;
            char detail[10240] = {0};
            trOne(infos[i]->bts[j], infos[i]->len[j], detail, sizeof(detail));
            fprintf(fpTxt, "#%d %s %s\n", j, infos[i]->bts[j], detail);
        }
        fprintf(fpTxt, "\n");
        //free(infos[i]);
    }
    fprintf(fpTxt, "\n");
    //free(infos);
    

    // printf tail
    while ((len=readLine(fpBt, line, sizeof(line))) > 0)
    {
        fprintf(fpTxt, "%s\n", line);
    }
}

static void trFile(char* bt, char* txt)
{
    FILE* fpBt = fopen(bt, "r");
    if (fpBt == 0)
    {
        int err = errno;
        printf("fopen(%s) fail[%s]", bt, strerror(err));
        return;
    }

    FILE* fpTxt = fopen(txt, "wt+");
    if (fpTxt == 0)
    {
        int err = errno;
        printf("fopen(%s) fail[%s]", txt, strerror(err));
        fclose(fpBt);
        return;
    }

    trBacktraces(fpBt, fpTxt);

    fclose(fpBt);
    fclose(fpTxt);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("usage: %s [input].bt [output].txt\n", argv[0]);
        return -1;
    }

    trFile(argv[1], argv[2]);
    return 0;
}

#define printf(fmt, ...)  do{sprintf(my_ret + my_len, fmt, ##__VA_ARGS__); my_len = strlen(my_ret);}while(0)
#define main myMain

#endif


