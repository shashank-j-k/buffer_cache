/*------------------------------------------------------------------------------------------------
    This program is an implementation of buffer cache and includes algorithms like :
    getblk
    brelse
--------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define QUEUES 4
#define MAX_BUFFER 40
#define PROCESSES 42

#define B_INVALID 1
#define B_BUSY 2
#define B_READ 4
#define B_ONDEMAND 8
#define B_DELAYEDWRITE 16

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
struct bufferheader *arArray[QUEUES];

pthread_mutex_t mLock;
pthread_cond_t mCondAny;

int main()
{
    int iCount;
    // struct bufferheader *pFLDummyHead = NULL;
    // struct bufferheader *arArray[QUEUES];
    struct bufferheader *pTemp = NULL;
    struct arg args[PROCESSES] = {{.iBlockNo = 27, .iDeviceNo = 1},
                                  {.iBlockNo = 28, .iDeviceNo = 1},
                                  {.iBlockNo = 29, .iDeviceNo = 1},
                                  {.iBlockNo = 30, .iDeviceNo = 1},
                                  {.iBlockNo = 23, .iDeviceNo = 1},
                                  {.iBlockNo = 24, .iDeviceNo = 1},
                                  {.iBlockNo = 25, .iDeviceNo = 1},
                                  {.iBlockNo = 22, .iDeviceNo = 1},
                                  {.iBlockNo = 21, .iDeviceNo = 1},
                                  {.iBlockNo = 3, .iDeviceNo = 1},
                                  {.iBlockNo = 39, .iDeviceNo = 1},
                                  {.iBlockNo = 33, .iDeviceNo = 1},
                                  {.iBlockNo = 17, .iDeviceNo = 1},
                                  {.iBlockNo = 18, .iDeviceNo = 1},
                                  {.iBlockNo = 19, .iDeviceNo = 1},
                                  {.iBlockNo = 10, .iDeviceNo = 1},
                                  {.iBlockNo = 7, .iDeviceNo = 1},
                                  {.iBlockNo = 4, .iDeviceNo = 1},
                                  {.iBlockNo = 5, .iDeviceNo = 1},
                                  {.iBlockNo = 6, .iDeviceNo = 1},
                                  {.iBlockNo = 1, .iDeviceNo = 1},
                                  {.iBlockNo = 40, .iDeviceNo = 1},
                                  {.iBlockNo = 9, .iDeviceNo = 1},
                                  {.iBlockNo = 8, .iDeviceNo = 1},
                                  {.iBlockNo = 15, .iDeviceNo = 1},
                                  {.iBlockNo = 31, .iDeviceNo = 1},
                                  {.iBlockNo = 36, .iDeviceNo = 1},
                                  {.iBlockNo = 32, .iDeviceNo = 1},
                                  {.iBlockNo = 12, .iDeviceNo = 1},
                                  {.iBlockNo = 35, .iDeviceNo = 1},
                                  {.iBlockNo = 38, .iDeviceNo = 1},
                                  {.iBlockNo = 37, .iDeviceNo = 1},
                                  {.iBlockNo = 11, .iDeviceNo = 1},
                                  {.iBlockNo = 13, .iDeviceNo = 1},
                                  {.iBlockNo = 34, .iDeviceNo = 1},
                                  {.iBlockNo = 2, .iDeviceNo = 1},
                                  {.iBlockNo = 14, .iDeviceNo = 1},
                                  {.iBlockNo = 16, .iDeviceNo = 1},
                                  {.iBlockNo = 20, .iDeviceNo = 1},
                                  {.iBlockNo = 26, .iDeviceNo = 1},
                                  {.iBlockNo = 22, .iDeviceNo = 1},
                                  {.iBlockNo = 30, .iDeviceNo = 1}};
    pthread_t tProcess[PROCESSES];

    // Mutex Initialization
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondAny, NULL);

    //  Initialization
    //==========================================================================================
    for (iCount = 0; iCount < QUEUES; iCount++)
    {
        arArray[iCount] = (struct bufferheader *)malloc(sizeof(struct bufferheader));
        if (NULL == arArray[iCount])
        {
            printf("Memory ALlocation Failed\n");
            return -1;
        }
        memset(arArray[iCount], 0, sizeof(struct bufferheader));
        arArray[iCount]->pBHQNext = arArray[iCount];
        arArray[iCount]->pBHQPrev = arArray[iCount];
        pthread_cond_init(&arArray[iCount]->mCondThat, NULL);
    }

    pFLDummyHead = (struct bufferheader *)malloc(sizeof(struct bufferheader));
    if (NULL == pFLDummyHead)
    {
        printf("Memory ALlocation Failed\n");
        return -1;
    }
    memset(pFLDummyHead, 0, sizeof(struct bufferheader));
    pFLDummyHead->pBFLNext = pFLDummyHead;
    pFLDummyHead->pBFLPrev = pFLDummyHead;
    pthread_cond_init(&pFLDummyHead->mCondThat, NULL);

    // StartUp: create buffers (all initially in hash queue 0) and add to freelist
    StartUp(arArray, &pFLDummyHead);

    iCount = CountNodes(arArray[0]);

    printf("[MAIN] Total buffers initialized in hash queue[0] = %d\n", iCount);
    //============================================================================================
    printf("Initial freelist and hashqueue\n");
    PrintFreeList();

    for (iCount = 0; iCount < QUEUES; iCount++)
    {
        printf("Hash Queue index [%d] :\n", iCount);
        PrintHashQueue(arArray[iCount]);
    }
    // Creating threads (processes calling getblk)
    for (iCount = 0; iCount < PROCESSES; iCount++)
    {
        if (pthread_create(&tProcess[iCount], NULL, &wrapper_getblk, (void *)&args[iCount]) != 0)
        {
            printf("Failed to create thread\n");
            return -1;
        }
    }
    // sleep(1);

    // Join threads and brelse returned buffers
    for (iCount = 0; iCount < PROCESSES; iCount++)
    {
        pthread_join(tProcess[iCount], (void **)&pTemp);
        if (pTemp != NULL)
        {
            printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
        }
        else
        {
            printf("[MAIN] getblk returned NULL");
        }

        sleep(1);
        if (pTemp != NULL)
            brelse(pTemp);
    }

    printf("Free List after getblk + brelse operations :\n");
    sleep(1);
    PrintFreeList();

    for (iCount = 0; iCount < QUEUES; iCount++)
    {
        printf("Hash Queue index [%d] :\n", iCount);
        PrintHashQueue(arArray[iCount]);
    }

    sleep(2);

    for (iCount = 0; iCount < QUEUES; iCount++)
    {
        pthread_cond_destroy(&arArray[iCount]->mCondThat);
        free(arArray[iCount]);
    }
    pthread_cond_destroy(&pFLDummyHead->mCondThat);
    free(pFLDummyHead);

    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondAny);

    DeleteAll();
    if(pFLDummyHead != NULL)
    {
        free(pFLDummyHead);
        pFLDummyHead = NULL;
    }
    printf("End of program\n");
    return 0;
}

//  StartUp: create MAX_BUFFER buffers, insert into hash queue [0] and freelist
void StartUp(struct bufferheader **ppHQPtr, struct bufferheader **ppFLHead)
{
    int iCounter;
    struct bufferheader *pReturn = NULL;
    printf("initializing buffers in hashqueue and freelist...\n");

    for (iCounter = 0; iCounter < MAX_BUFFER; iCounter++)
    {
        pReturn = InsertLast(ppHQPtr);
        if (NULL == pReturn)
        {
            printf("InsertLast Failed\n");
            exit(1);
        }

        InsertLastFreeList(ppFLHead, pReturn);
    }
}

// Create a new buffer node, init and insert into given hash head (tail)
struct bufferheader *InsertLast(struct bufferheader **ppHQHead)
{
    struct bufferheader *pNewNode = NULL;

    pNewNode = (struct bufferheader *)malloc(sizeof(struct bufferheader));
    if (pNewNode == NULL)
    {
        printf("Memory Allocation Failed\n");
        return NULL;
    }

    memset(pNewNode, 0, sizeof(struct bufferheader));
    pNewNode->iBlockNumber = -1;
    pNewNode->iDeviceNumber = -1;
    pNewNode->pDataPtr = NULL;
    pNewNode->pBFLNext = NULL;
    pNewNode->pBFLPrev = NULL;
    pNewNode->pBHQPrev = NULL;
    pNewNode->pBHQNext = NULL;
    pthread_cond_init(&pNewNode->mCondThat, NULL);

    if ((*ppHQHead)->pBHQNext == *ppHQHead)
    {
        (*ppHQHead)->pBHQNext = pNewNode;
        pNewNode->pBHQPrev = *ppHQHead;
    }

    else
    {
        pNewNode->pBHQPrev = (*ppHQHead)->pBHQPrev;
        (*ppHQHead)->pBHQPrev->pBHQNext = pNewNode;
    }
    pNewNode->pBHQNext = *ppHQHead;
    (*ppHQHead)->pBHQPrev = pNewNode;

    return pNewNode;
}

int CountNodes(struct bufferheader *pHead)
{
    int iCount = 0;
    struct bufferheader *pTemp = NULL;

    if (NULL == pHead)
        return 0;

    pTemp = pHead->pBHQNext;
    while (pTemp != pHead)
    {
        iCount++;
        pTemp = pTemp->pBHQNext;
    }

    return iCount;
}

// Insert a buffer at the end of freelist
void InsertLastFreeList(struct bufferheader **ppFLHead, struct bufferheader *pNewNode)
{
    if (*ppFLHead == NULL || pNewNode == NULL)
        return;

    if (pNewNode->pBFLNext != NULL && pNewNode->pBFLPrev != NULL)
        RemoveFromFreeList(pNewNode);

    if ((*ppFLHead)->pBFLNext == *ppFLHead && (*ppFLHead)->pBFLPrev == *ppFLHead) //
    {
        (*ppFLHead)->pBFLNext = pNewNode;
        pNewNode->pBFLPrev = *ppFLHead;
    }
    else
    {
        pNewNode->pBFLPrev = (*ppFLHead)->pBFLPrev;
        (*ppFLHead)->pBFLPrev->pBFLNext = pNewNode;
    }
    pNewNode->pBFLNext = *ppFLHead;
    (*ppFLHead)->pBFLPrev = pNewNode;
}

// Insert a buffer at the head of free list (MRU)
void InsertFirstFreeList(struct bufferheader **ppFLHead, struct bufferheader *pNewNode)
{
    if (*ppFLHead == NULL || pNewNode == NULL)
        return;

    if (pNewNode->pBFLNext != NULL && pNewNode->pBFLPrev != NULL)
        RemoveFromFreeList(pNewNode);

    pNewNode->pBFLNext = (*ppFLHead)->pBFLNext;
    (*ppFLHead)->pBFLNext->pBFLPrev = pNewNode;
    (*ppFLHead)->pBFLNext = pNewNode;
    pNewNode->pBFLPrev = *ppFLHead;
}

//  Insert a buffer at the end of a hashqueue
void InsertLastHashQueue(struct bufferheader *pHead, struct bufferheader *pNewNode)
{
    if (pHead == NULL || pNewNode == NULL)
        return;

    if (pHead->pBHQNext == pHead)
    {
        pHead->pBHQNext = pNewNode;
        pNewNode->pBHQPrev = pHead;
    }
    else
    {
        pNewNode->pBHQPrev = pHead->pBHQPrev;
        pHead->pBHQPrev->pBHQNext = pNewNode;
    }
    pNewNode->pBHQNext = pHead;
    pHead->pBHQPrev = pNewNode;
}

// Remove a buffer from freelist
void RemoveFromFreeList(struct bufferheader *pTemp)
{
    if (pTemp == NULL)
        return;

    if (pTemp->pBFLNext == NULL || pTemp->pBFLPrev == NULL)
        return;

    pTemp->pBFLPrev->pBFLNext = pTemp->pBFLNext;
    pTemp->pBFLNext->pBFLPrev = pTemp->pBFLPrev;
    pTemp->pBFLNext = NULL;
    pTemp->pBFLPrev = NULL;
}

void RemoveFromHashQueue(struct bufferheader *pTemp)
{
    if (pTemp == NULL)
        return;
    if (pTemp->pBHQNext == NULL || pTemp->pBHQPrev == NULL)
        return;
    pTemp->pBHQPrev->pBHQNext = pTemp->pBHQNext;
    pTemp->pBHQNext->pBHQPrev = pTemp->pBHQPrev;
    pTemp->pBHQNext = NULL;
    pTemp->pBHQPrev = NULL;
}

struct bufferheader *getblk(int iDeviceNo, int iBlockNo)
{
    // Raise processor interrupt execution level
    int iRem = iBlockNo % QUEUES;
    struct bufferheader *pTemp = NULL;

    pthread_mutex_lock(&mLock);
    while (1)
    {
        for (pTemp = arArray[iRem]->pBHQNext; pTemp != arArray[iRem]; pTemp = pTemp->pBHQNext)
        {
            if (pTemp->iBlockNumber == iBlockNo && pTemp->iDeviceNumber == iDeviceNo)
                break;
        }

        if (pTemp->iBlockNumber == iBlockNo && pTemp->iDeviceNumber == iDeviceNo) // block in hash queue
        {
            // *************scenario 5 - found buffer on hash queue but buffer is locked**********************
            if ((pTemp->chStatus & B_BUSY) != 0)
            {
                pTemp->chStatus |= B_ONDEMAND;
                printf("Buffer in use. Waiting for that buffer\n");
                // Sleep (event required buffer becomes free)
                pthread_cond_wait(&pTemp->mCondThat, &mLock);

                continue;
            }
            // *************scenario 1 - found buffer on hash queue*******************************************
            if (pTemp->pBFLNext != NULL && pTemp->pBFLPrev != NULL) // buffer is on free list
            {
                pTemp->chStatus |= B_BUSY; // marking buffer busy
                RemoveFromFreeList(pTemp);
            }

            printf("Buffer found in hash queue, locked and allocated\n");
            // lower processor interrupt execution level
            pthread_mutex_unlock(&mLock);

            return pTemp; // return buffer
        }
        else // block not on hash queue
        {
            // *************scenario 4 - no buffers on free list**********************************************
            if (pFLDummyHead->pBFLNext == pFLDummyHead) // no buffers on free list
            {
                printf("No buffers in freelist. Waiting for any buffer\n");
                // sleep (event any buffer becomes free)
                pthread_cond_wait(&mCondAny, &mLock);
                // pthread_mutex_unlock(&mLock);

                continue;
            }

            pTemp = pFLDummyHead->pBFLNext;

            pTemp->chStatus |= B_BUSY; // marking buffer busy
            RemoveFromFreeList(pTemp);

            // *************scenario 3 - found a free buffer on freelist but marked for delayed write******************

            if ((pTemp->chStatus & B_DELAYEDWRITE) != 0) // buffer marked for delayed write
            {
                printf("Buffer status delayed write on. Sent for Async write\n");
                // asynchronous write buffer to disk

                continue;
            }

            // *************scenario 2 - found a free buffer on freelist***************************************
            if (pTemp->pBHQNext != NULL && pTemp->pBHQPrev != NULL)
                RemoveFromHashQueue(pTemp);

            pTemp->iBlockNumber = iBlockNo;   // writing new block number on free buffer
            pTemp->iDeviceNumber = iDeviceNo; // writing new device number on free buffer

            pTemp->chStatus |= B_INVALID; // marking buffer invalid

            InsertLastHashQueue(arArray[iRem], pTemp); // Insert Buffer in new Hash Queue
            // lower processor interrupt execution level
            pthread_mutex_unlock(&mLock);

            return pTemp; // return buffer
        }
    }
}

void brelse(struct bufferheader *pTemp)
{
    if (pTemp == NULL)
        return;

    pthread_mutex_lock(&mLock);

    // 1. Mark buffer free first
    pTemp->chStatus &= ~B_BUSY;

    // Remove ONDEMAND flag
    if (pTemp->chStatus & B_ONDEMAND)
        pTemp->chStatus &= ~B_ONDEMAND;

    // 2. Insert into freelist
    if (((pTemp->chStatus & B_INVALID) == 0) &&
        ((pTemp->chStatus & B_DELAYEDWRITE) == 0))
        InsertLastFreeList(&pFLDummyHead, pTemp);
    else
        InsertFirstFreeList(&pFLDummyHead, pTemp);

    // 3. Wake threads waiting for ANY buffer
    pthread_cond_broadcast(&mCondAny);

    // 4. Wake threads waiting for THIS buffer
    pthread_cond_signal(&pTemp->mCondThat);

    printf("Buffer released: %d,%d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);

    pthread_mutex_unlock(&mLock);
}

// Priting the freelist
void PrintFreeList()
{
    int iCount = 1;

    if (NULL == pFLDummyHead)
    {
        printf("FreeList head missing\n");
        return;
    }

    struct bufferheader *pTemp = pFLDummyHead->pBFLNext;

    if (pTemp == pFLDummyHead)
    {
        printf("Free List is Empty\n");
        return;
    }

    while (pTemp != pFLDummyHead)
    {
        printf("FL[%d] - |Block No: %d, Device No: %d|\n", iCount++, pTemp->iBlockNumber, pTemp->iDeviceNumber);
        pTemp = pTemp->pBFLNext;
    }
}

// Printing the given hash queue
void PrintHashQueue(struct bufferheader *pHead)
{
    int iCount = 1;

    if (NULL == pHead)
    {
        printf("Hash queue head missing\n");
        return;
    }

    struct bufferheader *pTemp = pHead->pBHQNext;

    if (pTemp == pHead)
    {
        printf("Hash Queue is Empty\n");
        return;
    }

    while (pTemp != pHead)
    {
        printf("BHQ[%d] - |Block No: %d, Device No: %d|\n", iCount++, pTemp->iBlockNumber, pTemp->iDeviceNumber);
        pTemp = pTemp->pBHQNext;
    }
}

void *wrapper_getblk(void *args)
{
    struct arg *pPtr = (struct arg *)args;
    struct bufferheader *pReturn = getblk(pPtr->iDeviceNo, pPtr->iBlockNo);

    return (void *)pReturn;
}

void DeleteAll()
{
    int iCounter = 0;
    struct bufferheader *pTemp = NULL;

    for (iCounter = 0; iCounter < QUEUES; iCounter++)
    {
        if(arArray[iCounter] != NULL)
            arArray[iCounter]->pBHQPrev->pBHQNext = NULL;
        while (arArray[iCounter] != NULL)
        {
            pTemp = arArray[iCounter];
            arArray[iCounter] = arArray[iCounter]->pBHQNext;
            pTemp->pBHQNext = NULL;
            pTemp->pBHQPrev = NULL;
            free(pTemp);
            pTemp = NULL;
        }
    }
    // printf("Memory Cleared Successfully\n");
}