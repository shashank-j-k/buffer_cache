#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include "include/bufferheader.h"
#include "include/freelistheader.h"

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