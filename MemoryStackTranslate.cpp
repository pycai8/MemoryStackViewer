#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class MemoryStack 
{
public:
    unsigned long total;
    unsigned long count;
    vector<string> bts;
};

static string addr2line(const string& str)
{
    string cmd = "addr2line -Cfpie ";
    cmd += str;
    FILE* pp = popen(cmd.c_str(), "r");
    if (!pp)
    {
        return "";
    }

    char out[10240] = {0};
    fread(out, 1, sizeof(out) - 1, pp);
    pclose(pp);
    pp = 0;

    int len = strlen(out);
    if (len > 0 && out[len-1] == '\n')
    {
        out[len-1] = 0;
    }
    
    return string(out);
}

static string trOne(const string& str)
{
    static unordered_map<string, string> s_cache;

    auto it = s_cache.find(str);
    if (it != s_cache.end()) return it->second;

    s_cache[str] = addr2line(str);
    return s_cache[str];
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
    vector<MemoryStack*> mss;
    while ((len=readLine(fpBt, line, sizeof(line))) > 0)
    {    
        MemoryStack* pms = new MemoryStack();
        getTotalCount(line, len, &pms->total, &pms->count);
        while ((len=readLine(fpBt, line, sizeof(line))) > 0)
        {
            pms->bts.push_back(line);
        }
           
        mss.push_back(pms);
    }

    // sort
    for (int i = 0; i < mss.size(); i++)
    {
        int max = i;
        for (int j = i+1; j < mss.size(); j++)
        {
            if (mss[j]->total > mss[max]->total) max = j;
        }
        // swap max and i
        MemoryStack* tmp = mss[i];
        mss[i] = mss[max];
        mss[max] = tmp;
    }

    // print body
    fprintf(fpTxt, "\n\n");
    for (int i = 0; i < mss.size(); i++)
    {
        fprintf(fpTxt, "total:%lu, count: %lu\n", mss[i]->total, mss[i]->count);
        for (int j = 0; j < mss[i]->bts.size(); j++)
        {
            string str = trOne(mss[i]->bts[j]);
            fprintf(fpTxt, "#%d %s %s\n", j, mss[i]->bts[j].c_str(), str.c_str());
        }
        fprintf(fpTxt, "\n");
        delete mss[i];
        mss[i] = 0;
    }
    fprintf(fpTxt, "\n");
    

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


