#include <stdio.h>
#include <pthread.h>
#include "include/bufferheader.h"
#include "include/freelistheader.h"

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
