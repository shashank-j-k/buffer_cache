#include <stdio.h>
#include <malloc.h>

struct arg
{
    int iDeviceNo;
    int iBlockNo;
};

void StartUp(struct bufferheader **, struct bufferheader **);
struct bufferheader *InsertLast(struct bufferheader **);
int CountNodes(struct bufferheader *);
void InsertLastFreeList(struct bufferheader **, struct bufferheader *);
void InsertLastHashQueue(struct bufferheader *, struct bufferheader *);
struct bufferheader *getblk(int, int);
void RemoveFromFreeList(struct bufferheader *);
void RemoveFromHashQueue(struct bufferheader *);
void PrintFreeList();
void PrintHashQueue(struct bufferheader *);
void InsertFirstFreeList(struct bufferheader **, struct bufferheader *);
void brelse(struct bufferheader *);
void *wrapper_getblk(void *);
void DeleteAll();

struct bufferheader *pFLDummyHead = NULL;

