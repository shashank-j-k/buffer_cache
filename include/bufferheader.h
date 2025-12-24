#pragma once
#include <stdio.h>
#include <pthread.h>

#define B_INVALID 1
#define B_BUSY 2
#define B_READ 4
#define B_ONDEMAND 8
#define B_DELAYEDWRITE 16

#define QUEUES 4
#define MAX_BUFFER 40
#define PROCESSES 42

struct bufferheader
{
    int iDeviceNumber;
    int iBlockNumber;
    unsigned char chStatus; // 0 - valid/invalid, 1 - free/locked, 2 - read/write, 3 - on demand, 4 - delayed write
    char *pDataPtr;
    struct bufferheader *pBFLNext;
    struct bufferheader *pBFLPrev;
    struct bufferheader *pBHQNext;
    struct bufferheader *pBHQPrev;
    pthread_cond_t mCondThat;
};

struct bufferheader *arArray[QUEUES];

pthread_mutex_t mLock;
pthread_cond_t mCondAny;