/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum {
   INVALID = 0,
   SHARED = 1,
   MODIFIED = 2
};

enum {
   BusRd = 1,
   BusRdX = 2,
   BusUpgr = 3
};

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()                { tag = 0; Flags = 0; }
   ulong getTag()             { return tag; }
   ulong getFlags()           { return Flags;}
   ulong getSeq()             { return seq; }
   void setSeq(ulong Seq)     { seq = Seq;}
   void setFlags(ulong flags) {  Flags = flags;}
   void setTag(ulong a)       { tag = a; }
   void invalidate()          { tag = 0; Flags = INVALID; } //useful function
   bool isValid()             { return ((Flags) != INVALID); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   
   ulong flushes,interventions,invalidations,nobusrdx,nobusupgr,c2ctransfer;

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)   { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag) { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM()     {return readMisses;} 
   ulong getWM()     {return writeMisses;} 
   ulong getReads()  {return reads;}       
   ulong getWrites() {return writes;}
   ulong getWB()     {return writeBacks;}
   
   void writeBack(ulong) {writeBacks++;}
   void flush() {flushes++;}
   void incrinterventions() {interventions++;}
   void incrinvalidations() {invalidations++;}
   void incrc2ctrans() {c2ctransfer++;}
   void incrbusrdx() {nobusrdx++;}
   void incrbusupgr() {nobusupgr++;}


   ulong Access(ulong,uchar,ulong);
   void Snoop(ulong,ulong);
   void printStats(ulong);
   void updateLRU(cacheLine *);

   //******///
   //add other functions to handle bus transactions///
   //******///

};

#endif
