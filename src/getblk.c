#include <stdio.h>
#include <pthread.h>

#include "include/bufferheader.h"
#include "include/freelistheader.h"

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

void *wrapper_getblk(void *args)
{
    struct arg *pPtr = (struct arg *)args;
    struct bufferheader *pReturn = getblk(pPtr->iDeviceNo, pPtr->iBlockNo);

    return (void *)pReturn;
}