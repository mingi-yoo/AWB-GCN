#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

using namespace std;

class Cache {
public:
	Cache();
	~Cache();

	void Insert(uint64_t address);
	bool Access(uint64_t address);
private:
	// sa_cache[set][way]
	vector<vector<uint64_t>> sa_cache;

	int FindSet(uint64_t address);
	int FindWay(uint64_t address, int set);
};

#endif