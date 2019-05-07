//
// Created by Bar on 27-Apr-19.
//

#ifndef HW2_COMPUTERARCH_CACHE_H
#define HW2_COMPUTERARCH_CACHE_H

#include <vector>
#include <cstdint>
#include <cmath>
#include <cassert>

#define POW2(exp) ((uint32_t)(1 << (exp)))
#define NOT_FOUND (-1)
#define NO_ADDRESS ((uint32_t)~0)

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
		tag_t tagBits;
        bool validBit;
        int lruCount;
    //    bool m_dirtyBit;
    };

    struct Set {
        std::vector<TagLine> tags;
       // std::vector<Block> blocks;
    };

    //enum WRITE_POLICY {
	//	NO_WRITE_ALLOC = 0, WRITE_ALLOC = 1
    //};

	int m_numOfAccesses;
	int m_hits;
	int m_totalTime;
    uint32_t m_cacheSize;
	uint32_t m_blockSize; //offset bits
	uint32_t m_numOfWays;
	uint32_t m_numOfCycles;

	uint32_t m_numOfSetBits; //pow 2 = number of sets
	uint32_t m_numOfOffsetBits;
	uint32_t m_numOfTagBits;

	bool m_fifo; //true if fifo policy

    
	//WRITE_POLICY m_writePolicy;

    std::vector<Set> m_sets;
    

    address_t m_tagMask;
	address_t m_setMask;
	address_t m_offsetMask;

public:

    Cache(unsigned int cacheSize, unsigned int blockSize, unsigned int assoc,
          unsigned int cycles //, unsigned int writePolicy
	) :
		m_cacheSize(cacheSize), m_blockSize(blockSize), m_numOfWays(assoc),
		m_numOfCycles(cycles), m_numOfAccesses(0), m_hits(0){

    	m_numOfOffsetBits = blockSize;

		//m_writePolicy = WRITE_POLICY(writePolicy);
		m_numOfSetBits = cacheSize - blockSize;
		m_numOfSetBits = m_numOfOffsetBits -assoc;
		m_numOfTagBits = 32 - (m_blockSize + m_numOfSetBits);

		m_offsetMask = POW2(m_numOfOffsetBits) - 1;

		m_setMask = (POW2(m_numOfSetBits + m_numOfOffsetBits) - 1);

		m_tagMask = ~(m_setMask);
    }

	//if victim cache don't update LRU if found. otherwise update.
	wayIdx_t Find(uint32_t address) {
		m_numOfAccesses++;
		m_totalTime += m_numOfCycles;
    	set_t set = GetSet(address);
		tag_t tag = GetTag(address);

		for (wayIdx_t way = 0; static_cast<wayIdx_t>(m_numOfWays); way++) {
			TagLine& cacheline = m_sets[set].tags[way];
			if (cacheline.validBit && cacheline.tagBits == tag) {
				if (!m_fifo) {
					UpdateLRU(set, way);
				}
				m_hits++;
				return way;
			}
		}
		return NOT_FOUND;
	}

	//let's say that numOfWays-1 is the most reacently used, and min count is the victim.
	void UpdateLRU(uint32_t address) {
        set_t set = GetSet(address);
        tag_t tag = GetTag(address);
        wayIdx_t way = NO_ADDRESS;
        for(wayIdx_t i = 0; i < static_cast<wayIdx_t>(m_numOfWays); i++){
            TagLine& cacheline = m_sets[set].tags[i];
            if(cacheline.validBit && cacheline.tagBits == tag){
                way = i;
            }
        }
        assert(way != NO_ADDRESS);
        UpdateLRU(set, way);
	}

	void UpdateLRU(set_t set, wayIdx_t way) {
        int x = m_sets[set].tags[way].lruCount;
        m_sets[set].tags[way].lruCount = m_numOfWays -1;
        for(int i = 0; i < static_cast<wayIdx_t>(m_numOfWays); i++){
            TagLine& cacheline = m_sets[set].tags[i];
            if(cacheline.validBit && cacheline.tagBits != m_sets[set].tags[way].tagBits && cacheline.lruCount > x){
                cacheline.lruCount--;
            }
        }
	}

	wayIdx_t findEmptyWay(set_t set) {
		for (wayIdx_t way = 0; way < static_cast<wayIdx_t>(m_numOfWays); way++) {
			if (m_sets[set].tags[way].validBit == false) {
				return way;
			}
		}
		return NOT_FOUND;
	}

	//return removed
	address_t RemoveLast(set_t set) {
		assert(findEmptyWay(set) == NOT_FOUND);
		TagLine& victim = m_sets[set].tags[0];
		for (TagLine& cacheLine : m_sets[set].tags ) {
			if (cacheLine.lruCount < victim.lruCount) {
				victim = cacheLine;
			}
		}
		victim.validBit = false;
		return GetAddress(set, victim.tagBits);
	}

	//returns address that was removed or NO_ADDRESS if non removed. and update LRU
	address_t Add(uint32_t address) {

		set_t set = GetSet(address);

		wayIdx_t empty = findEmptyWay(set);
		address_t victim = NO_ADDRESS;
		if (empty == NOT_FOUND) {
			victim = RemoveLast(set);
		}
		empty = findEmptyWay(set);
		assert(empty != NOT_FOUND);

		m_sets[set].tags[empty].tagBits = GetTag(address);
		assert(!m_sets[set].tags[empty].validBit);
		m_sets[set].tags[empty].validBit = true;
		UpdateLRU(set, empty);

		return victim;

	}

	int GetTotalCycles() { return m_totalTime; }

	float getHitRate(){
    	return float(m_hits)/float(m_numOfAccesses);
    }


    set_t GetSet(address_t address){
        address &= m_setMask;
        return address>> m_numOfTagBits;
    }

	address_t GetAddress(set_t set, tag_t tag) {

	}

	tag_t GetTag(address_t address) {
		return address & m_tagMask;
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
