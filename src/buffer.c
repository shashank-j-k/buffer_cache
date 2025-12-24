#include <stdio.h>
#include "include/bufferheader.h"
#include "include/freelistheader.h"

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

// Remove a buffer from Hash Queue
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
