/////////////////include///////////////////////
#define _GNU_SOURCE
#include <dlfcn.h>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

////////////////pre-defines///////////////////////
#define RPT_SIGNAL 64
#define MAX_STACK_LEN 32
#define RPT_DIR "/var/log/msv"
#define BKT_CNT (0xffff+1)
#define VID_MV_CNT 48

#define DBG(fmt, ...) //{printf("debug %s:%d %s => ", __FILE__, __LINE__, __func__);printf(fmt, ##__VA_ARGS__);printf("\n");}
#define INFO(fmt, ...) //{printf("info %s:%d %s => ", __FILE__, __LINE__, __func__);printf(fmt, ##__VA_ARGS__);printf("\n");}
#define ERR(fmt, ...) //{printf("error %s:%d %s => ", __FILE__, __LINE__, __func__);printf(fmt, ##__VA_ARGS__);printf("\n");}
#define RPT(fp, fmt, ...) {fprintf(fp, fmt, ##__VA_ARGS__);fprintf(fp, "\n");}

///////////////pfn type defines///////////////////////
typedef void* (*PFN_Malloc)(size_t);
typedef void (*PFN_Free)(void*);
typedef void* (*PFN_Calloc)(size_t, size_t);
typedef void* (*PFN_Realloc)(void*, size_t);
typedef void* (*PFN_Valloc)(size_t);
typedef void* (*PFN_PValloc)(size_t);
typedef void* (*PFN_MemAlign)(size_t, size_t);
typedef void* (*PFN_LibcMemAlign)(size_t, size_t);
typedef int (*PFN_PosixMemAlign)(void**, size_t, size_t);
typedef void* (*PFN_AlignedAlloc)(size_t, size_t);
typedef void (*PFN_SignalHandler)(int);
typedef PFN_SignalHandler (*PFN_Signal)(int, PFN_SignalHandler);

/////////////stack struct defines///////////////////
typedef struct tagMemBt
{
    struct tagMemBt* next;
    
    unsigned long count;
    unsigned long total;

    unsigned long depth;
    unsigned long vid;
    void* bt[MAX_STACK_LEN];
} MemBt;

typedef struct tagPtrHdr
{
    MemBt* stack;
    unsigned long size;
} PtrHdr;

#define HDR_LEN sizeof(PtrHdr)
#define ADD_HDR(ptr) ((unsigned char*)ptr - HDR_LEN)
#define CUT_HDR(ptr) ((unsigned char*)ptr + HDR_LEN)

//////////////////global values///////////////////
static int g_init = 0;
static __thread int g_innerCall = 0;
static MemBt* g_list[BKT_CNT] = {0};
static unsigned long g_count = 0;
static unsigned long g_total = 0;
static unsigned long g_chc = 0; // crc hint count

static PFN_Malloc g_pfnMalloc = 0;
static PFN_Free g_pfnFree = 0;
static PFN_Calloc g_pfnCalloc = 0;
static PFN_Realloc g_pfnRealloc = 0;
static PFN_Valloc g_pfnValloc = 0;
static PFN_PValloc g_pfnPValloc = 0;
static PFN_MemAlign g_pfnMemAlign = 0;
static PFN_LibcMemAlign g_pfnLibcMemAlign = 0;
static PFN_PosixMemAlign g_pfnPosixMemAlign = 0;
static PFN_AlignedAlloc g_pfnAlignedAlloc = 0;
static PFN_Signal g_pfnSignal = 0;

static PFN_SignalHandler g_pfnRptSigOutHdr = SIG_DFL;

static char g_pidCmd[1024] = {0};

static const unsigned long long g_crc64Table[256] = {
    0x0000000000000000ULL, 0x7ad870c830358979ULL,
    0xf5b0e190606b12f2ULL, 0x8f689158505e9b8bULL,
    0xc038e5739841b68fULL, 0xbae095bba8743ff6ULL,
    0x358804e3f82aa47dULL, 0x4f50742bc81f2d04ULL,
    0xab28ecb46814fe75ULL, 0xd1f09c7c5821770cULL,
    0x5e980d24087fec87ULL, 0x24407dec384a65feULL,
    0x6b1009c7f05548faULL, 0x11c8790fc060c183ULL,
    0x9ea0e857903e5a08ULL, 0xe478989fa00bd371ULL,
    0x7d08ff3b88be6f81ULL, 0x07d08ff3b88be6f8ULL,
    0x88b81eabe8d57d73ULL, 0xf2606e63d8e0f40aULL,
    0xbd301a4810ffd90eULL, 0xc7e86a8020ca5077ULL,
    0x4880fbd87094cbfcULL, 0x32588b1040a14285ULL,
    0xd620138fe0aa91f4ULL, 0xacf86347d09f188dULL,
    0x2390f21f80c18306ULL, 0x594882d7b0f40a7fULL,
    0x1618f6fc78eb277bULL, 0x6cc0863448deae02ULL,
    0xe3a8176c18803589ULL, 0x997067a428b5bcf0ULL,
    0xfa11fe77117cdf02ULL, 0x80c98ebf2149567bULL,
    0x0fa11fe77117cdf0ULL, 0x75796f2f41224489ULL,
    0x3a291b04893d698dULL, 0x40f16bccb908e0f4ULL,
    0xcf99fa94e9567b7fULL, 0xb5418a5cd963f206ULL,
    0x513912c379682177ULL, 0x2be1620b495da80eULL,
    0xa489f35319033385ULL, 0xde51839b2936bafcULL,
    0x9101f7b0e12997f8ULL, 0xebd98778d11c1e81ULL,
    0x64b116208142850aULL, 0x1e6966e8b1770c73ULL,
    0x8719014c99c2b083ULL, 0xfdc17184a9f739faULL,
    0x72a9e0dcf9a9a271ULL, 0x08719014c99c2b08ULL,
    0x4721e43f0183060cULL, 0x3df994f731b68f75ULL,
    0xb29105af61e814feULL, 0xc849756751dd9d87ULL,
    0x2c31edf8f1d64ef6ULL, 0x56e99d30c1e3c78fULL,
    0xd9810c6891bd5c04ULL, 0xa3597ca0a188d57dULL,
    0xec09088b6997f879ULL, 0x96d1784359a27100ULL,
    0x19b9e91b09fcea8bULL, 0x636199d339c963f2ULL,
    0xdf7adabd7a6e2d6fULL, 0xa5a2aa754a5ba416ULL,
    0x2aca3b2d1a053f9dULL, 0x50124be52a30b6e4ULL,
    0x1f423fcee22f9be0ULL, 0x659a4f06d21a1299ULL,
    0xeaf2de5e82448912ULL, 0x902aae96b271006bULL,
    0x74523609127ad31aULL, 0x0e8a46c1224f5a63ULL,
    0x81e2d7997211c1e8ULL, 0xfb3aa75142244891ULL,
    0xb46ad37a8a3b6595ULL, 0xceb2a3b2ba0eececULL,
    0x41da32eaea507767ULL, 0x3b024222da65fe1eULL,
    0xa2722586f2d042eeULL, 0xd8aa554ec2e5cb97ULL,
    0x57c2c41692bb501cULL, 0x2d1ab4dea28ed965ULL,
    0x624ac0f56a91f461ULL, 0x1892b03d5aa47d18ULL,
    0x97fa21650afae693ULL, 0xed2251ad3acf6feaULL,
    0x095ac9329ac4bc9bULL, 0x7382b9faaaf135e2ULL,
    0xfcea28a2faafae69ULL, 0x8632586aca9a2710ULL,
    0xc9622c4102850a14ULL, 0xb3ba5c8932b0836dULL,
    0x3cd2cdd162ee18e6ULL, 0x460abd1952db919fULL,
    0x256b24ca6b12f26dULL, 0x5fb354025b277b14ULL,
    0xd0dbc55a0b79e09fULL, 0xaa03b5923b4c69e6ULL,
    0xe553c1b9f35344e2ULL, 0x9f8bb171c366cd9bULL,
    0x10e3202993385610ULL, 0x6a3b50e1a30ddf69ULL,
    0x8e43c87e03060c18ULL, 0xf49bb8b633338561ULL,
    0x7bf329ee636d1eeaULL, 0x012b592653589793ULL,
    0x4e7b2d0d9b47ba97ULL, 0x34a35dc5ab7233eeULL,
    0xbbcbcc9dfb2ca865ULL, 0xc113bc55cb19211cULL,
    0x5863dbf1e3ac9decULL, 0x22bbab39d3991495ULL,
    0xadd33a6183c78f1eULL, 0xd70b4aa9b3f20667ULL,
    0x985b3e827bed2b63ULL, 0xe2834e4a4bd8a21aULL,
    0x6debdf121b863991ULL, 0x1733afda2bb3b0e8ULL,
    0xf34b37458bb86399ULL, 0x8993478dbb8deae0ULL,
    0x06fbd6d5ebd3716bULL, 0x7c23a61ddbe6f812ULL,
    0x3373d23613f9d516ULL, 0x49aba2fe23cc5c6fULL,
    0xc6c333a67392c7e4ULL, 0xbc1b436e43a74e9dULL,
    0x95ac9329ac4bc9b5ULL, 0xef74e3e19c7e40ccULL,
    0x601c72b9cc20db47ULL, 0x1ac40271fc15523eULL,
    0x5594765a340a7f3aULL, 0x2f4c0692043ff643ULL,
    0xa02497ca54616dc8ULL, 0xdafce7026454e4b1ULL,
    0x3e847f9dc45f37c0ULL, 0x445c0f55f46abeb9ULL,
    0xcb349e0da4342532ULL, 0xb1eceec59401ac4bULL,
    0xfebc9aee5c1e814fULL, 0x8464ea266c2b0836ULL,
    0x0b0c7b7e3c7593bdULL, 0x71d40bb60c401ac4ULL,
    0xe8a46c1224f5a634ULL, 0x927c1cda14c02f4dULL,
    0x1d148d82449eb4c6ULL, 0x67ccfd4a74ab3dbfULL,
    0x289c8961bcb410bbULL, 0x5244f9a98c8199c2ULL,
    0xdd2c68f1dcdf0249ULL, 0xa7f41839ecea8b30ULL,
    0x438c80a64ce15841ULL, 0x3954f06e7cd4d138ULL,
    0xb63c61362c8a4ab3ULL, 0xcce411fe1cbfc3caULL,
    0x83b465d5d4a0eeceULL, 0xf96c151de49567b7ULL,
    0x76048445b4cbfc3cULL, 0x0cdcf48d84fe7545ULL,
    0x6fbd6d5ebd3716b7ULL, 0x15651d968d029fceULL,
    0x9a0d8ccedd5c0445ULL, 0xe0d5fc06ed698d3cULL,
    0xaf85882d2576a038ULL, 0xd55df8e515432941ULL,
    0x5a3569bd451db2caULL, 0x20ed197575283bb3ULL,
    0xc49581ead523e8c2ULL, 0xbe4df122e51661bbULL,
    0x3125607ab548fa30ULL, 0x4bfd10b2857d7349ULL,
    0x04ad64994d625e4dULL, 0x7e7514517d57d734ULL,
    0xf11d85092d094cbfULL, 0x8bc5f5c11d3cc5c6ULL,
    0x12b5926535897936ULL, 0x686de2ad05bcf04fULL,
    0xe70573f555e26bc4ULL, 0x9ddd033d65d7e2bdULL,
    0xd28d7716adc8cfb9ULL, 0xa85507de9dfd46c0ULL,
    0x273d9686cda3dd4bULL, 0x5de5e64efd965432ULL,
    0xb99d7ed15d9d8743ULL, 0xc3450e196da80e3aULL,
    0x4c2d9f413df695b1ULL, 0x36f5ef890dc31cc8ULL,
    0x79a59ba2c5dc31ccULL, 0x037deb6af5e9b8b5ULL,
    0x8c157a32a5b7233eULL, 0xf6cd0afa9582aa47ULL,
    0x4ad64994d625e4daULL, 0x300e395ce6106da3ULL,
    0xbf66a804b64ef628ULL, 0xc5bed8cc867b7f51ULL,
    0x8aeeace74e645255ULL, 0xf036dc2f7e51db2cULL,
    0x7f5e4d772e0f40a7ULL, 0x05863dbf1e3ac9deULL,
    0xe1fea520be311aafULL, 0x9b26d5e88e0493d6ULL,
    0x144e44b0de5a085dULL, 0x6e963478ee6f8124ULL,
    0x21c640532670ac20ULL, 0x5b1e309b16452559ULL,
    0xd476a1c3461bbed2ULL, 0xaeaed10b762e37abULL,
    0x37deb6af5e9b8b5bULL, 0x4d06c6676eae0222ULL,
    0xc26e573f3ef099a9ULL, 0xb8b627f70ec510d0ULL,
    0xf7e653dcc6da3dd4ULL, 0x8d3e2314f6efb4adULL,
    0x0256b24ca6b12f26ULL, 0x788ec2849684a65fULL,
    0x9cf65a1b368f752eULL, 0xe62e2ad306bafc57ULL,
    0x6946bb8b56e467dcULL, 0x139ecb4366d1eea5ULL,
    0x5ccebf68aecec3a1ULL, 0x2616cfa09efb4ad8ULL,
    0xa97e5ef8cea5d153ULL, 0xd3a62e30fe90582aULL,
    0xb0c7b7e3c7593bd8ULL, 0xca1fc72bf76cb2a1ULL,
    0x45775673a732292aULL, 0x3faf26bb9707a053ULL,
    0x70ff52905f188d57ULL, 0x0a2722586f2d042eULL,
    0x854fb3003f739fa5ULL, 0xff97c3c80f4616dcULL,
    0x1bef5b57af4dc5adULL, 0x61372b9f9f784cd4ULL,
    0xee5fbac7cf26d75fULL, 0x9487ca0fff135e26ULL,
    0xdbd7be24370c7322ULL, 0xa10fceec0739fa5bULL,
    0x2e675fb4576761d0ULL, 0x54bf2f7c6752e8a9ULL,
    0xcdcf48d84fe75459ULL, 0xb71738107fd2dd20ULL,
    0x387fa9482f8c46abULL, 0x42a7d9801fb9cfd2ULL,
    0x0df7adabd7a6e2d6ULL, 0x772fdd63e7936bafULL,
    0xf8474c3bb7cdf024ULL, 0x829f3cf387f8795dULL,
    0x66e7a46c27f3aa2cULL, 0x1c3fd4a417c62355ULL,
    0x935745fc4798b8deULL, 0xe98f353477ad31a7ULL,
    0xa6df411fbfb21ca3ULL, 0xdc0731d78f8795daULL,
    0x536fa08fdfd90e51ULL, 0x29b7d047efec8728ULL
};

//////////////////time pid cmd crc/////////////////

static void getCurTime(char* out, int len)
{
    time_t cur = time(0);
    struct tm now = {0};
    localtime_r(&cur, &now);
    
    now.tm_year += 1900;
    now.tm_mon += 1;

    snprintf(out, len, "time_%d%s%d%s%d_%s%d%s%d%s%d"
        , now.tm_year
        , (now.tm_mon < 10 ? "0" : "")
        , now.tm_mon
        , (now.tm_mday < 10 ? "0" : "")
        , now.tm_mday
        , (now.tm_hour < 10 ? "0" : "")
        , now.tm_hour
        , (now.tm_min < 10 ? "0" : "")
        , now.tm_min
        , (now.tm_sec < 10 ? "0" : "")
        , now.tm_sec
        );

}

static void initPidCmd()
{
    if (g_pidCmd[0] != 0) return;

    pid_t pid = getpid();

    char file[100] = {0};
    snprintf(file, sizeof(file), "/proc/%lu/cmdline", (unsigned long)pid);
    FILE* fp = fopen(file, "r");
    if (fp == 0)
    {
        return;
    }

    char cmd[1024] = {0};
    fread(cmd, sizeof(cmd), 1, fp);
    fclose(fp);

    int i = 0;
    for (i = 0; i < sizeof(cmd) && cmd[i] != 0; i++)
    {
        if ('0' < cmd[i] && cmd[i] < '9') continue;
        if ('a' < cmd[i] && cmd[i] < 'z') continue;
        if ('A' < cmd[i] && cmd[i] < 'Z') continue;
        cmd[i] = '_';
    }

    snprintf(g_pidCmd, sizeof(g_pidCmd), "pid_%lu_cmdline_%s", (unsigned long)pid, cmd);
}

static unsigned long long getVid(void** bt, unsigned long depth)
{
    unsigned long long crc = 0;

    int i = 0;
    for (i = 0; i < depth; i++)
    {
        unsigned long ptrVal = (unsigned long)bt[i];
        unsigned char* ptrTmp = (unsigned char*)&ptrVal;
        int j = 0;
        for (j = 0; j < sizeof(ptrVal); j++) 
        {
            unsigned char byte = ptrTmp[j];
            crc = g_crc64Table[(crc ^ byte) & 0xff] ^ (crc >> 8);
        }
    }

    return crc; 
}

//////////////////stack functions//////////////////////

static void rptSigHdr(int sigNum)
{
    if (sigNum != RPT_SIGNAL) return;

    if (g_pfnRptSigOutHdr != SIG_ERR && g_pfnRptSigOutHdr != 0) g_pfnRptSigOutHdr(sigNum);

    char time[100] = {0};
    getCurTime(time, sizeof(time));
    char file[1024] = {0};
    snprintf(file, sizeof(file), "%s/%s_%s.bt", RPT_DIR, time, g_pidCmd);

    FILE* fp = fopen(file, "at+");
    if (fp == 0)
    {
        ERR("open file[%s] fail[%s]", file, strerror(errno));
        return;
    }

    INFO("open file[%s] success", file);

    RPT(fp,
        "%s\n" // time
        "%s\n" // pid and cmdline
        "count: %lu, total: %lu\n" // whole count and whole total
        , time
        , g_pidCmd 
        , g_count, g_total);

    int i = 0;
    unsigned long totalBtCnt = 0;
    unsigned long usingBtCnt = 0;
    unsigned long selfUseMem = 0;
    unsigned long maxBtCnt = 0;
    unsigned long maxUsingBtCnt = 0;
    for (i = 0; i < BKT_CNT; i++)
    {
        MemBt* tmp = 0;
        unsigned long tmpBtCnt = 0;
        unsigned long tmpUsingBtCnt = 0;
        for (tmp = g_list[i]; tmp != 0; tmp = tmp->next)
        {
            DBG("vid: %lu", tmp->vid);
            totalBtCnt++;
            tmpBtCnt++;
            if (tmp->count == 0 || tmp->total == 0) continue;
            usingBtCnt++;
            tmpUsingBtCnt++;
            RPT(fp, "\ncount: %lu, total: %lu", tmp->count, tmp->total);
            int j = 0;
            for (j = 0; j < tmp->depth; j++)
            {
                void* ptr = tmp->bt[j];
                Dl_info info = {0};
                if (dladdr(ptr, &info) == 0)
                {
                    RPT(fp, "? ?");
                    continue;
                }
                unsigned long diff = (unsigned long)ptr;
                if (diff >> 32 != 0 && info.dli_fbase != 0) diff -= (unsigned long)info.dli_fbase; 
                RPT(fp,
                    "%s 0x%x"
                    , ((info.dli_fname != 0 && *info.dli_fname != 0) ? info.dli_fname : "?")
                    , diff
                    );
            }
        }
        if (tmpBtCnt > maxBtCnt) maxBtCnt = tmpBtCnt;
        if (tmpUsingBtCnt > maxUsingBtCnt) maxUsingBtCnt = tmpUsingBtCnt;
    }
    selfUseMem = totalBtCnt * sizeof(MemBt) + sizeof(g_list);
    RPT(fp,
        "\n\ntotal bt count: %lu"
        "\nusing bt count: %lu"
        "\nself use memory: %lu"
        "\ncrc hit count: %lu"
        "\nmax bucket bt count: %lu"
        "\nmax bucket using bt count: %lu"
        , totalBtCnt
        , usingBtCnt
        , selfUseMem
        , g_chc
        , maxBtCnt
        , maxUsingBtCnt);

    fclose(fp);
}

static MemBt* stackGetCurrent(size_t size)
{
    MemBt* ret = (MemBt*)malloc(sizeof(MemBt));
    if (ret == 0) return 0;

    ret->next = 0;
    ret->count = 1;
    ret->total = size;
    
    ret->depth = backtrace(ret->bt, MAX_STACK_LEN);
    ret->vid = getVid(ret->bt, ret->depth);
    return ret;
}

static int stackEqual(MemBt* bt1, MemBt* bt2)
{
    if (bt1->vid != bt2->vid) return 0;

    if (bt1->depth != bt2->depth)
    {
        DBG("crc hit when diff depth");
        __sync_add_and_fetch(&g_chc, 1);
        return 0;
    }

    int i = 0;
    for (i = 0; i < bt1->depth; i++)
    {
        if (bt1->bt[i] != bt2->bt[i])
        {
            __sync_add_and_fetch(&g_chc, 1);
            DBG("crc hit when diff bt");
            return 0;
        }
    }

    return 1;
}

static void* stackMalloc(void* ptr, size_t size)
{
    PtrHdr* hdr = (PtrHdr*)ptr;
    void* ret = CUT_HDR(ptr);
    hdr->size = size - HDR_LEN;
    hdr->stack = 0;

    __sync_add_and_fetch(&g_count, 1);
    __sync_add_and_fetch(&g_total, hdr->size);

    MemBt* stack = stackGetCurrent(hdr->size);
    if (stack == 0) return ret;

    unsigned long bktId = stack->vid >> VID_MV_CNT;
    MemBt* tmp = __sync_val_compare_and_swap(&g_list[bktId], 0, stack);
    if (tmp == 0)
    {
        hdr->stack = stack;
        return ret;
    }

    while(1)
    {
        if (stackEqual(tmp, stack))
        {
            __sync_add_and_fetch(&tmp->count, stack->count);
            __sync_add_and_fetch(&tmp->total, stack->total);
            free(stack);
            hdr->stack = tmp;
            return ret;
        }

        tmp = __sync_val_compare_and_swap(&tmp->next, 0, stack);
        if (tmp == 0)
        {
            hdr->stack = stack;
            return ret;
        }
    }

}

static void stackFree(void* ptr)
{
    PtrHdr* hdr = (PtrHdr*)ptr;
    __sync_sub_and_fetch(&g_count, 1);
    __sync_sub_and_fetch(&g_total, hdr->size);
    
    if (hdr->stack == 0) return;
    __sync_sub_and_fetch(&hdr->stack->count, 1);
    __sync_sub_and_fetch(&hdr->stack->total, hdr->size);
}

//////////////////outter functions/////////////////////
__attribute__ ((constructor))
static void init()
{
    if (!g_init)
    {
        g_init = 1;
        // do here
        g_pfnMalloc = (PFN_Malloc)dlsym(RTLD_NEXT, "malloc");
        g_pfnFree = (PFN_Free)dlsym(RTLD_NEXT, "free");
        g_pfnCalloc = (PFN_Calloc)dlsym(RTLD_NEXT, "calloc");
        g_pfnRealloc = (PFN_Realloc)dlsym(RTLD_NEXT, "realloc");
        g_pfnValloc = (PFN_Valloc)dlsym(RTLD_NEXT, "valloc");
        g_pfnPValloc = (PFN_PValloc)dlsym(RTLD_NEXT, "pvalloc");
        g_pfnMemAlign = (PFN_MemAlign)dlsym(RTLD_NEXT, "memalign");
        g_pfnLibcMemAlign = (PFN_LibcMemAlign)dlsym(RTLD_NEXT, "libc_memalign");
        g_pfnPosixMemAlign = (PFN_PosixMemAlign)dlsym(RTLD_NEXT, "posix_memalign");
        g_pfnAlignedAlloc = (PFN_AlignedAlloc)dlsym(RTLD_NEXT, "aligned_alloc");
        g_pfnSignal = (PFN_Signal)dlsym(RTLD_NEXT, "signal");
        PFN_SignalHandler ret = SIG_ERR;
        if (g_pfnSignal) ret = g_pfnSignal(RPT_SIGNAL, rptSigHdr);
        if (ret == SIG_ERR) ERR("report signal[%d] install fail", RPT_SIGNAL);
        initPidCmd();
    }
}

__attribute__ ((destructor))
static void uninit()
{
    if (g_init)
    {
        // do here
        g_init = 0;
    }
}

void* malloc(size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnMalloc == 0 ? 0 : g_pfnMalloc(size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = malloc(size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

void free(void* ptr)
{
    init();

    if (ptr == 0) return;

    if (g_innerCall) { if (g_pfnFree) g_pfnFree(ptr); return; }

    g_innerCall = 1;
    stackFree(ADD_HDR(ptr));
    g_innerCall = 0;
}

void* calloc(size_t cnt, size_t size)
{
    init();

    if (cnt == 0 || size == 0) return 0;

    if (g_innerCall) return (g_pfnCalloc == 0 ? 0 : g_pfnCalloc(cnt, size));

    void* ret = 0;
    g_innerCall = 1;

    cnt *= size;
    cnt += HDR_LEN;
    ret = calloc(cnt, 1);
    if (ret != 0) ret = stackMalloc(ret, cnt);

    g_innerCall = 0;
    return ret;
}

void* realloc(void* ptr, size_t size)
{
    init();

    if (ptr == 0 && size == 0) return 0;
    if (ptr == 0 && size != 0) return malloc(size);
    if (ptr != 0 && size == 0) { free(ptr); return 0; }

    if (g_innerCall) return (g_pfnRealloc == 0 ? 0 : g_pfnRealloc(ptr, size));

    void* ret = 0;
    g_innerCall = 1;

    ptr = ADD_HDR(ptr);
    size += HDR_LEN;
    PtrHdr oldHdr = {0};
    oldHdr = *(PtrHdr*)ptr;
    ret = realloc(ptr, size);
    if (ret != 0) { stackFree((void*)&oldHdr); ret = stackMalloc(ret, size); }

    g_innerCall = 0;
    return ret;
}

void* valloc(size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnValloc == 0 ? 0 : g_pfnValloc(size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = valloc(size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

void* pvalloc(size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnPValloc == 0 ? 0 : g_pfnPValloc(size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = pvalloc(size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

void* memalign(size_t alignment, size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnMemAlign == 0 ? 0 : g_pfnMemAlign(alignment, size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = memalign(alignment, size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

void* libc_memalign(size_t alignment, size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnLibcMemAlign == 0 ? 0 : g_pfnLibcMemAlign(alignment, size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = libc_memalign(alignment, size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

int posix_memalign(void** ptr, size_t alignment, size_t size)
{
    init();

    if (ptr == 0) return EINVAL;
    if (size == 0) { *ptr = 0; return EINVAL; }

    if (g_innerCall) return (g_pfnPosixMemAlign == 0 ? 0 : g_pfnPosixMemAlign(ptr, alignment, size));

    int ret = EINVAL;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = posix_memalign(ptr, alignment, size);
    if (ret != 0) *ptr = 0;
    else if (*ptr == 0) ret = EINVAL;
    else *ptr = stackMalloc(*ptr, size);

    g_innerCall = 0;
    return ret;
}

void* aligned_alloc(size_t alignment, size_t size)
{
    init();

    if (size == 0) return 0;

    if (g_innerCall) return (g_pfnAlignedAlloc == 0 ? 0 : g_pfnAlignedAlloc(alignment, size));

    void* ret = 0;
    g_innerCall = 1;

    size += HDR_LEN;
    ret = aligned_alloc(alignment, size);
    if (ret != 0) ret = stackMalloc(ret, size);

    g_innerCall = 0;
    return ret;
}

PFN_SignalHandler signal(int sigNum, PFN_SignalHandler sigHdr)
{
    init();

    if (!(1 <= sigNum && sigNum <= 64)) return SIG_ERR;
    if (sigHdr == 0) return SIG_ERR; // fixme??? 0 means clear handler???

    if (g_innerCall) return (g_pfnSignal == 0 ? SIG_ERR : g_pfnSignal(sigNum, sigHdr));

    PFN_SignalHandler ret = SIG_ERR;
    g_innerCall = 1;

    if (sigNum != RPT_SIGNAL) ret = signal(sigNum, sigHdr);
    else ret = __sync_lock_test_and_set(&g_pfnRptSigOutHdr, sigHdr);

    g_innerCall = 0;
    return ret;
}

