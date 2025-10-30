/*------------------------------------------------------------------------------------------------
    getblk
    This algorithm
--------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define QUEUES 4
#define MAX_BUFFER 40
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
};

void StartUp(struct bufferheader **, struct bufferheader **);
struct bufferheader *InsertLast(struct bufferheader **, struct bufferheader **);
int CountNodes(struct bufferheader *);
void InsertLastFreeList(struct bufferheader **, struct bufferheader *);
void InsertLastHashQueue(struct bufferheader *, struct bufferheader *);
struct bufferheader *getblk(int, int);
void RemoveFromFreeList(struct bufferheader *);
void RemoveFromHashQueue(struct bufferheader *);
void PrintFreeList(struct bufferheader *);
void PrintHashQueue(struct bufferheader *);
void InsertFirstFreeList(struct bufferheader **, struct bufferheader *);
void brelse(struct bufferheader *);

struct bufferheader *pFLDummyHead = NULL;
struct bufferheader *arArray[QUEUES];

int main()
{
    int iCount;
    // struct bufferheader *pFLDummyHead = NULL;
    // struct bufferheader *arArray[QUEUES];
    struct bufferheader *pTemp = NULL;

    //  Initialization
    //==========================================================================================
    for (iCount = 0; iCount < QUEUES; iCount++)
    {
        arArray[iCount] = (struct bufferheader *)malloc(sizeof(struct bufferheader));
        memset(arArray[iCount], 0, sizeof(struct bufferheader));
        arArray[iCount]->pBHQNext = arArray[iCount];
        arArray[iCount]->pBHQPrev = arArray[iCount];
    }

    pFLDummyHead = (struct bufferheader *)malloc(sizeof(struct bufferheader));
    memset(pFLDummyHead, 0, sizeof(struct bufferheader));
    pFLDummyHead->pBFLNext = pFLDummyHead;
    pFLDummyHead->pBFLPrev = pFLDummyHead;

    StartUp(arArray, &pFLDummyHead);
    iCount = CountNodes(arArray[0]);

    printf("Total buffers initialized = %d\n", iCount);
    //============================================================================================

    pTemp = getblk(1, 28);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }

    pTemp = getblk(1, 27);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }

    pTemp = getblk(1, 26);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }

    pTemp = getblk(1, 25);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }

    pTemp = getblk(1, 24);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }

    printf("Free List after getblk :\n");
    PrintFreeList(pFLDummyHead);

    printf("Hash Queue index [%d] :\n", 28 % QUEUES);
    PrintHashQueue(arArray[28 % QUEUES]);
    printf("Hash Queue index [%d] :\n", 1 % QUEUES);
    PrintHashQueue(arArray[1 % QUEUES]);
    printf("Hash Queue index [%d] :\n", 2 % QUEUES);
    PrintHashQueue(arArray[2 % QUEUES]);
    printf("Hash Queue index [%d] :\n", 3 % QUEUES);
    PrintHashQueue(arArray[3 % QUEUES]);

    brelse(pTemp);

    pTemp = getblk(1, 24);
    if (pTemp != NULL)
    {
        printf("getblk allocated buffer : device no  : %d, block no : %d\n", pTemp->iDeviceNumber, pTemp->iBlockNumber);
    }
    else
        printf("getblk returned NULL");
    printf("Freelist after brelse\n");
    PrintFreeList(pFLDummyHead);

    return 0;
}

void StartUp(struct bufferheader **ppPtr, struct bufferheader **ppHead)
{
    int iCounter;
    struct bufferheader *pTemp = NULL;
    printf("initializing Buffers in hashqueue and freelist\n");

    for (iCounter = 0; iCounter < MAX_BUFFER; iCounter++)
    {
        // printf("%d\n", iCounter);
        pTemp = InsertLast(ppPtr, ppHead);
        if (pTemp == NULL)
        {
            printf("InsertLast Failed\n");
            exit(1);
        }

        InsertLastFreeList(ppHead, pTemp);
    }
}

struct bufferheader *InsertLast(struct bufferheader **ppHead, struct bufferheader **ppFLHead)
{
    int iCounter;
    struct bufferheader *pNewNode = NULL;

    pNewNode = (struct bufferheader *)malloc(sizeof(struct bufferheader));
    if (pNewNode == NULL)
    {
        printf("Memory Allocation Failed\n");
        return NULL;
    }

    memset(pNewNode, 0, sizeof(struct bufferheader)); // ***********
    pNewNode->iBlockNumber = -1;
    pNewNode->iDeviceNumber = -1;
    pNewNode->pDataPtr = NULL;

    if ((*ppHead)->pBHQNext == *ppHead)
    {
        (*ppHead)->pBHQNext = pNewNode;
        pNewNode->pBHQPrev = *ppHead;
    }

    else
    {
        pNewNode->pBHQPrev = (*ppHead)->pBHQPrev;
        (*ppHead)->pBHQPrev->pBHQNext = pNewNode;
    }
    pNewNode->pBHQNext = *ppHead;
    (*ppHead)->pBHQPrev = pNewNode;

    return pNewNode;
}

int CountNodes(struct bufferheader *pHead)
{
    int iCount = 0;
    struct bufferheader *pTemp = NULL;

    if (pHead == NULL)
        return 0;

    pTemp = pHead->pBHQNext;
    while (pTemp != pHead)
    {
        iCount++;
        pTemp = pTemp->pBHQNext;
    }

    return iCount;
}

void InsertLastFreeList(struct bufferheader **ppHead, struct bufferheader *pNewNode)
{
    if (ppHead == NULL || pNewNode == NULL)
        return;

    if ((*ppHead)->pBFLNext == *ppHead)
    {
        (*ppHead)->pBFLNext = pNewNode;
        pNewNode->pBFLPrev = *ppHead;
    }
    else
    {
        pNewNode->pBFLPrev = (*ppHead)->pBFLPrev;
        (*ppHead)->pBFLPrev->pBFLNext = pNewNode;
    }
    pNewNode->pBFLNext = *ppHead;
    (*ppHead)->pBFLPrev = pNewNode;
}

struct bufferheader *getblk(int iDeviceNo, int iBlockNo)
{
    // Raise processor interrupt execution level
    int iRem = iBlockNo % QUEUES;
    struct bufferheader *pTemp = NULL;

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
                continue;
            }
            // *************scenario 1 - found buffer on hash queue*******************************************
            if (pTemp->pBFLNext != NULL) // buffer is on free list
            {
                pTemp->chStatus |= B_BUSY; // marking buffer busy
                RemoveFromFreeList(pTemp);
            }

            printf("Buffer found in hash queue, locked and allocated\n");
            // lower processor interrupt execution level

            return pTemp; // return buffer
        }
        else // block not on hash queue
        {
            // *************scenario 4 - no buffers on free list**********************************************
            if (pFLDummyHead->pBFLNext == pFLDummyHead) // no buffers on free list
            {
                printf("No buffers in freelist. Waiting for any buffer\n");
                // sleep (event any buffer becomes free)
                continue;
            }

            pTemp = pFLDummyHead->pBFLNext;

            pTemp->chStatus |= B_BUSY; // marking buffer busy
            RemoveFromFreeList(pTemp);

            if ((pTemp->chStatus & B_DELAYEDWRITE) != 0) // buffer marked for delayed write
            {
                printf("Buffer status delayed write on. Sent for Async write\n");
                // asynchronous write buffer to disk
                continue;
            }

            // *************scenario 2 - found a free buffer on freelist***************************************
            RemoveFromHashQueue(pTemp);

            pTemp->iBlockNumber = iBlockNo;   // writing new block number on free buffer
            pTemp->iDeviceNumber = iDeviceNo; // writing new device number on free buffer

            pTemp->chStatus |= B_INVALID; // marking buffer invalid

            InsertLastHashQueue(arArray[iRem], pTemp); // Insert Buffer in new Hash Queue
            // lower processor interrupt execution level

            return pTemp; // return buffer
        }
    }
}

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

void RemoveFromFreeList(struct bufferheader *pTemp)
{
    if (pTemp == NULL)
        return;

    if (pTemp->pBFLNext == NULL || pTemp->pBFLPrev == NULL)
        return;

    pTemp->pBFLPrev->pBFLNext = pTemp->pBFLNext; // removing buffer from free list
    pTemp->pBFLNext->pBFLPrev = pTemp->pBFLPrev; // removing buffer from free list
    pTemp->pBFLNext = NULL;                      // removing buffer from free list
    pTemp->pBFLPrev = NULL;                      // removing buffer from free list
}

void RemoveFromHashQueue(struct bufferheader *pTemp)
{
    if (pTemp == NULL)
        return;
    if (pTemp->pBHQNext == NULL || pTemp->pBHQPrev == NULL)
        return;
    pTemp->pBHQPrev->pBHQNext = pTemp->pBHQNext; // removing buffer from old hashqueue
    pTemp->pBHQNext->pBHQPrev = pTemp->pBHQPrev; // removing buffer from old hashqueue
    pTemp->pBHQNext = NULL;                      // removing buffer from old hashqueue
    pTemp->pBHQPrev = NULL;                      // removing buffer from old hashqueue
}

void PrintFreeList(struct bufferheader *pHead)
{
    int iCount = 1;
    struct bufferheader *pTemp = pHead->pBFLNext;

    if (pTemp == pHead)
    {
        printf("Free List is Empty\n");
        return;
    }

    while (pTemp != pHead)
    {
        printf("FL[%d] - |Block No: %d, Device No: %d|\n", iCount++, pTemp->iBlockNumber, pTemp->iDeviceNumber);
        pTemp = pTemp->pBFLNext;
    }
}

void PrintHashQueue(struct bufferheader *pHead)
{
    int iCount = 1;
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

void brelse(struct bufferheader *pTemp)
{
    // wake up processes waiting for any buffer
    // wake up processes waiting for that buffer

    // raise processor interrupt execution level

    if (((pTemp->chStatus & B_INVALID) == 0) && ((pTemp->chStatus & B_DELAYEDWRITE) == 0))
        InsertLastFreeList(&pFLDummyHead, pTemp);
    else
        InsertFirstFreeList(&pFLDummyHead, pTemp);

    pTemp->chStatus &= ~B_BUSY;
    printf("Buffer released successfully and added to freelist\n");
    // lower processor interrupt execution level
}

void InsertFirstFreeList(struct bufferheader **ppHead, struct bufferheader *pNewNode)
{
    if (*ppHead == NULL || pNewNode == NULL)
        return;

    pNewNode->pBFLNext = (*ppHead)->pBFLNext;
    (*ppHead)->pBFLNext->pBFLPrev = pNewNode;
    (*ppHead)->pBFLNext = pNewNode;
    pNewNode->pBFLPrev = *ppHead;
}
