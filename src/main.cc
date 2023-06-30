/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
    
    ifstream fin;
    FILE * pFile;

    if(argv[1] == NULL){
         printf("input format: ");
         printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
         exit(0);
        }
    ulong BusSignal;
    ulong i;
    ulong cache_size     = atoi(argv[1]);
    ulong cache_assoc    = atoi(argv[2]);
    ulong blk_size       = atoi(argv[3]);
    ulong num_processors = atoi(argv[4]);
    ulong protocol       = atoi(argv[5]); /* 0:MSI 1:MSI BusUpgr 2:MESI 3:MESI Snoop FIlter */
    char *fname        = (char *) malloc(20);
    fname              = argv[6];

    //printing configurations
    printf("===== 506 Coherence Simulator Configuration =====\n");
    printf("L1_SIZE: %lu\n",cache_size);
    printf("L1_ASSOC: %lu\n",cache_assoc);
    printf("L1_BLOCKSIZE: %lu\n",blk_size);
    printf("NUMBER OF PROCESSORS: %lu\n",num_processors);
    printf("COHERENCE PROTOCOL: ");
    switch(protocol)
    {
        case 0: printf("MSI\n"); break;
        case 1: printf("MSI BusUpgr\n"); break;
        case 2: printf("MESI\n"); break;
        case 3: printf("MESI Filter\n"); break;
    }
    printf("TRACE FILE: %s\n",fname); //NO PROBLEM TILL HERE
    // Using pointers so that we can use inheritance */
    Cache** cacheArray = (Cache **) malloc(num_processors * sizeof(Cache));
    for(ulong i = 0; i < num_processors; i++) {
        if(protocol == 0) {
            cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size);
        }
    }

    pFile = fopen (fname,"r");
    if(pFile == 0)
    {   
        printf("Trace file problem\n");
        exit(0);
    }
    
    ulong proc;
    char op;
    ulong addr;

    int line = 1;
    while(fscanf(pFile, "%lu %c %lx", &proc, &op, &addr) != EOF)
    {
#ifdef _DEBUG
        //printf("%d\n", line);
#endif
        //printf("inside while loop\n");
        // propagate request down through memory hierarchy
        // by calling cachesArray[processor#]->Access(...)
        //printf("accessing access() function\n");
        BusSignal = cacheArray[proc]->Access(addr,op,protocol);
        if(BusSignal != 0)
        {
            for(i=0;i<num_processors;i++)
            {
                if(i != proc)
                {
                    cacheArray[i]->Snoop(BusSignal,addr);
                }
            }
        } 
        line++;
    }

    fclose(pFile);

    for (i = 0; i < num_processors; i++)
    {
        cacheArray[i]->printStats(i);
    }
    
    
}
