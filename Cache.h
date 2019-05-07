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

enum WRITE_POLICY {
	WRITE_ALLOC = 0,
	NO_WRITE_ALLOC,
};

enum OPERATION {
	READ = 0,
	WRITE,
};

class Cache {

    struct TagLine {
		tag_t tagBits;
        bool validBit;
        int lruCount;
        bool dirtyBit;
    };

    struct Set {
        std::vector<TagLine> tags;
    };

	int m_numOfAccesses;
	int m_hits;
    uint32_t m_cacheSize;
	uint32_t m_blockSize; //offset bits
	uint32_t m_numOfWays;
	int m_numOfCycles;

	uint32_t m_numOfSetBits; //pow 2 = number of sets
	uint32_t m_numOfOffsetBits;
	uint32_t m_numOfTagBits;

	bool m_fifo; //true if fifo policy

    std::vector<Set> m_sets;

    address_t m_tagMask;
	address_t m_setMask;
	address_t m_offsetMask;

public:

    Cache(unsigned int cacheSize, unsigned int blockSize, unsigned int assoc,
          unsigned int cycles) :
		m_cacheSize(cacheSize), m_blockSize(blockSize), m_numOfWays(assoc),
		m_numOfCycles(cycles), m_numOfAccesses(0), m_hits(0){

    	m_numOfOffsetBits = blockSize;

		m_numOfSetBits = cacheSize - blockSize;
		m_numOfSetBits = m_numOfOffsetBits -assoc;
		m_numOfTagBits = 32 - (m_blockSize + m_numOfSetBits);

		m_offsetMask = POW2(m_numOfOffsetBits) - 1;

		m_setMask = (POW2(m_numOfSetBits + m_numOfOffsetBits) - 1);

		m_tagMask = ~(m_setMask);
    }

	//if victim cache - don't update LRU if found. otherwise update.
	wayIdx_t Find(uint32_t address, OPERATION op) {
		m_numOfAccesses++;
    	set_t set = GetSet(address);
		tag_t tag = GetTag(address);

		for (wayIdx_t way = 0; static_cast<wayIdx_t>(m_numOfWays); way++) {
			TagLine& cacheline = m_sets[set].tags[way];
			if (cacheline.validBit && cacheline.tagBits == tag) {
				if (!m_fifo) {
					UpdateLRU(set, way);
				}
				m_hits++;
				if (op == WRITE) {
					cacheline.dirtyBit = true;
				}
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

	//return removed, return if it was dirty.
	address_t RemoveLast(set_t set, bool& dirtyRemove) {
		assert(findEmptyWay(set) == NOT_FOUND);
		TagLine& victim = m_sets[set].tags[0];
		for (TagLine& cacheLine : m_sets[set].tags ) {
			if (cacheLine.lruCount < victim.lruCount) {
				victim = cacheLine;
			}
		}
		victim.validBit = false;
		dirtyRemove = victim.dirtyBit;
		return GetAddress(set, victim.tagBits);
	}

	//returns address that was removed or NO_ADDRESS if non removed. and update LRU
	address_t Add(uint32_t address, bool& dirtyRemove) {

		set_t set = GetSet(address);

		wayIdx_t empty = findEmptyWay(set);
		address_t victim = NO_ADDRESS;
		if (empty == NOT_FOUND) {
			victim = RemoveLast(set, dirtyRemove);
		}
		empty = findEmptyWay(set);
		assert(empty != NOT_FOUND);

		m_sets[set].tags[empty].tagBits = GetTag(address);
		assert(!m_sets[set].tags[empty].validBit);
		m_sets[set].tags[empty].validBit = true;
		UpdateLRU(set, empty);

		return victim;

	}

	int GetTotalCycles() { return m_numOfAccesses*m_numOfCycles; }

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

	int m_memAccess;

	bool m_usingVictimCache;

	WRITE_POLICY m_writePolicy;

	CacheSim() {}

	void AddToL1(address_t adr) {} //TODO
	void AddToL2(address_t adr) {} //TODO

	void HandleNewAddress(address_t adr, char operation) {
		OPERATION op = (operation == 'W') ? WRITE : READ;
		bool noAlloc = (op == WRITE && m_writePolicy == WRITE_POLICY::NO_WRITE_ALLOC);

		if (m_L1.Find(adr, op) != NOT_FOUND) {
			return; // not need for anything else.
		}

		if (m_L2.Find(adr, op) != NOT_FOUND) {

			if ((op == WRITE && m_writePolicy == WRITE_ALLOC) || op == READ) {
				AddToL1(adr);
			}
			return;
		}

		if (m_usingVictimCache) {
			if (m_victim.Find(adr, op) != NOT_FOUND) {
				AddToL2(adr);
				AddToL1(adr);
				return;
			}
		}
		m_memAccess++;
		if ((op == WRITE && m_writePolicy == WRITE_ALLOC) || op == READ) {
			AddToL2(adr); //first add to L2.
			AddToL1(adr);
		}
		return;
	}
};




#endif //HW2_COMPUTERARCH_CACHE_H
