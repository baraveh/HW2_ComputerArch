//
// Created by Bar on 27-Apr-19.
//

#ifndef HW2_COMPUTERARCH_CACHE_H
#define HW2_COMPUTERARCH_CACHE_H

#include <vector>
#include <cstdint>
#include <cmath>

#define POW2(exp) ((uint32_t)(1 << (exp)))
#define NOT_FOUND (-1)

typedef int wayIdx_t;
typedef uint32_t set_t;
typedef uint32_t tag_t;
typedef uint32_t address_t;

class Cache {

    /* struct Block {
        //std::vector<uint32_t> dataArr_m;
    };
    */

    struct TagLine {
		tag_t m_tagBits;
        bool m_validBit;
        int m_LRUCount;
    //    bool m_dirtyBit;
    };

    struct Way {
        std::vector<TagLine> m_tags;
       // std::vector<Block> blocks_m;
    };

    //enum WRITE_POLICY {
	//	NO_WRITE_ALLOC = 0, WRITE_ALLOC = 1
    //};

    uint32_t m_cacheSize;
	uint32_t m_blockSize; //offset bits
	uint32_t m_numOfWays;
	uint32_t m_numOfCycles;

	uint32_t m_numOfSetBits; //pow 2 = number of sets
	uint32_t m_numOfOffsetBits;
	uint32_t m_numOfTagBits;

	bool m_fifo; //true if fifo policy

    
	//WRITE_POLICY m_writePolicy;

    std::vector<Way> m_ways;
    

    address_t m_tagMask;
	address_t m_setMask;
	address_t m_offsetMask;

public:

    Cache(unsigned int cacheSize, unsigned int blockSize, unsigned int assoc,
          unsigned int cycles //, unsigned int writePolicy
	) :
		m_cacheSize(cacheSize), m_blockSize(blockSize), m_numOfWays(assoc),
		m_numOfCycles(cycles){

		//m_writePolicy = WRITE_POLICY(writePolicy);
		m_numOfSetBits = cacheSize - blockSize;
		m_numOfSetBits = m_numOfOffsetBits -assoc;
		m_numOfTagBits = 32 - (m_blockSize + m_numOfSetBits);

		m_offsetMask = POW2(m_numOfOffsetBits) - 1;

		m_setMask = (POW2(m_numOfSetBits + m_numOfOffsetBits) - 1);

		m_tagMask = ~(m_setMask);
    }

	//if victim don't update LRU if found. otherwise update.
	wayIdx_t Find(uint32_t address) {
	
	}

	bool UpdateLRU(uint32_t address) {

	}

	bool UpdateLRU(set_t set, wayIdx_t way) {

	}

	//return removed
	address_t RemoveLast() {

	}

	//returns address that was removed or -1 if non removed. and update LRU
	address_t Add(uint32_t address) {

	}

	uint32_t GetMissCycles() { return m_numOfCycles; }



    set_t getSetNum(uint32_t address){
        address &= m_setMask;
        return address>> m_numOfTagBits;
    }

};


class CacheSim {

	//more simulator params
	
	
	Cache m_L1;
	Cache m_L2;
	Cache m_victim;

	int m_totalTime;
	int m_L1Misses;
	int m_L2Misses;
	int m_totalAccess;

	CacheSim(  ) {}

	bool L1Search(address_t adr) {
		m_totalTime += m_l1AccessTime;
		//update miss/hit rat
		if (m_L1.Find(adr) != NOT_FOUND) {
			m_L2.UpdateLRU(adr);
			return true;
		}
		return false;
	}
	bool L2Search(address_t adr) {
		m_totalTime += m_l2AccessTime;
		//update miss/hit rate
		if (m_L2.Find(adr) != NOT_FOUND) {
			m_L1.Add(adr);
			return true;
		}
		return false;
	}
	bool VictimSearch(address_t adr) {
		if (!m_withVictimCache) {
			return false;
		}
		m_totalTime += m_victimAccessTime;
		if (m_victim.Find(adr) != NOT_FOUND) {
			m_L1.Add(adr);
			m_L2.Add(adr);
			return true;
		}
		return false;
	}

	void HandleNewAddress(address_t adr, char operation) {
		if (L1Search(adr)) {
			return;
		}
		if (L2Search(adr)) {
			return;
		}
		if (VictimSearch(adr)) {
			return;
		}
		m_totalTime += m_memAccessTime;
		if (operation == 'W' && m_writePolicy == NO_WRITE_ALLOC) {
			return;
		}
		m_L1.Add(adr);
		address_t victimAddress = m_L2.Add(adr);
		if (victimAddress != NOT_FOUND) {
			m_victim.Add(victimAddress);
		}		
	}
};


#endif //HW2_COMPUTERARCH_CACHE_H
