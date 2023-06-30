/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
      tagMask <<= 1;
      tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
         cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
ulong Cache::Access(ulong addr,uchar op, ulong protocol)
{
   //printf("inside the function, incrementing cycle\n");
   currentCycle++;/*per cache global counter to maintain LRU order 
                    among cache ways, updated on every cache access*/  
   //printf("cycles incremented\n");
   if(op == 'w') writes++;
   else          reads++;
   cacheLine * line = findLine(addr);
   if(line == NULL)/*miss*/
   {
      if(op == 'w') writeMisses++;
      else readMisses++;
      cacheLine *newline = fillLine(addr);
      if(op == 'w')
      {
         newline->setFlags(MODIFIED);
         incrbusrdx();
         return BusRdX;
      }
      else return BusRd;     
      
   }
   
   else
   {
      /**since it's a hit, update LRU and update dirty flag**/
      updateLRU(line);
      if(op == 'w')
      {
         if(line->getFlags() == SHARED)
         {
            line->setFlags(MODIFIED);
            if(protocol == 0)
            {
               incrbusrdx();
               return BusRdX;
            }

         
            if(protocol == 1)
            {
               incrbusupgr();
               return BusUpgr;
            }
            
         }
         //line->setFlags(MODIFIED);
      }
      return 0;
   }
}

void Cache::Snoop(ulong BusSignal, ulong addr)
{
   cacheLine * line = findLine(addr);
   if(line != NULL)
   {
      switch (BusSignal)
      {
         case BusRd:
            if(line->getFlags() == MODIFIED)
            {
               line->setFlags(SHARED);
               flush();
               incrinterventions();
            }
            break;
         
         case BusRdX:
            if(line->getFlags() == MODIFIED)
            {
               flush();
            }
            line->setFlags(INVALID);
            incrinvalidations();
            break;

         case BusUpgr:
            line->setFlags(INVALID);
            incrinvalidations();
            break;
      }
   }
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
   if(cache[i][j].isValid()) {
      if(cache[i][j].getTag() == tag)
      {
         pos = j; 
         break; 
      }
   }
   if(pos == assoc) {
      return NULL;
   }
   else {
      return &(cache[i][pos]); 
   }
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
   line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) { 
         return &(cache[i][j]); 
      }   
   }

   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].getSeq() <= min) { 
         victim = j; 
         min = cache[i][j].getSeq();}
   } 

   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   
   if(victim->getFlags() == MODIFIED) {
      writeBack(addr);
   }
      
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(SHARED);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats(ulong current_proc)
{ 
   printf("============ Simulation results (Cache %lu) ============\n",current_proc);
   printf("01. number of reads: %lu\n", reads);
   printf("02. number of read misses: %lu\n", readMisses);
   printf("03. number of writes: %lu\n", writes);
   printf("04. number of write misses: %lu\n", writeMisses);
   printf("05. total miss rate: %.2lf%%\n", 100*((double)(readMisses+writeMisses)/(double)(reads+writes)));
   printf("06. number of writebacks: %lu\n", writeBacks);
   printf("07. number of cache-to-cache transfers: %lu\n", c2ctransfer);
   printf("08. number of memory transactions: %lu\n", (readMisses+writeBacks+nobusrdx));
   printf("09. number of interventions: %lu\n", interventions);
   printf("10. number of invalidations: %lu\n", invalidations);
   printf("11. number of flushes: %lu\n", flushes);
   printf("12. number of BusRdX: %lu\n", nobusrdx);
   printf("13. number of BusUpgr: %lu\n", nobusupgr);
}
