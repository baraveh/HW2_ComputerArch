//
// Created by Bar on 27-Apr-19.
//

#ifndef HW2_COMPUTERARCH_CACHE_H
#define HW2_COMPUTERARCH_CACHE_H

#include <vector>
#include <cstdint>
#include <cmath>

#define POW2(exp) ((uint32_t)(1 << (exp)))

class Cache {

    /* struct Block {
        //std::vector<uint32_t> dataArr_m;
    };
    */

    struct TagLine {
        uint32_t tagBits_m;
        bool validBit_m;
        int LRUCount_m;
        bool dirtyBit;
    };

    struct Way {
        std::vector<TagLine> tags_m;
       // std::vector<Block> blocks_m;
    };

    enum WritePolicy {
        WriteThrough = 0, WriteBack = 1
    };

    unsigned int cacheSize_m;
    unsigned int blockSize_m; //offset bits
    unsigned int numOfWays_m;
    unsigned int numOfCycles_m;

    unsigned int setBits_m; //pow 2 = number of sets
    unsigned int offsetBits_m;
    unsigned int tagBits_m;

    
    WritePolicy writePolicy_m;

    std::vector<Way> ways_m;
    

    uint32_t tagMask;
    uint32_t setMask;
    uint32_t offsetMask;




    Cache(unsigned int cacheSize, unsigned int blockSize, unsigned int assoc,
          unsigned int cycles, unsigned int writePolicy) :
          cacheSize_m(cacheSize), blockSize_m(blockSize), numOfWays_m(assoc),
          numOfCycles_m(cycles){

        writePolicy_m = WritePolicy(writePolicy);
        offsetBits_m = cacheSize - blockSize;
        setBits_m = offsetBits_m-assoc;
        tagBits_m = 32 - (blockSize_m + setBits_m);

        offsetMask = POW2(offsetBits_m) - 1;

        setMask = (POW2(setBits_m+offsetBits_m) - 1);

        tagMask = ~(setMask);


    }

    unsigned int getSetNum(uint32_t address){
        address &= setMask;
        return address>>tagBits_m;
    }


};


#endif //HW2_COMPUTERARCH_CACHE_H
