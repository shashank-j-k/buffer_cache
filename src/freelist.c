#include <stdio.h>
#include "include/bufferheader.h"
#include "include/freelistheader.h"

// Insert a buffer at the end of freelist
void InsertLastFreeList(struct bufferheader **ppFLHead, struct bufferheader *pNewNode)
{
    if (*ppFLHead == NULL || pNewNode == NULL)
        return;

    if (pNewNode->pBFLNext != NULL && pNewNode->pBFLPrev != NULL)
        RemoveFromFreeList(pNewNode);

    if ((*ppFLHead)->pBFLNext == *ppFLHead && (*ppFLHead)->pBFLPrev == *ppFLHead) 
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

