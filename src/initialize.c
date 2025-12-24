#include <stdio.h>
#include <malloc.h>
#include "include/bufferheader.h"
#include "include/freelistheader.h"

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