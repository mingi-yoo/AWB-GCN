#include "cache.h"

using namespace std;

extern ArchInfo arch_info;

Cache::Cache() {
	for (int i = 0; i < arch_info.cache_set; i++)
		sa_cache.push_back(vector<uint64_t> ());
}

Cache::~Cache() {}

void Cache::Insert(uint64_t address) {
	int set = FindSet(address);

	if (sa_cache[set].size() == arch_info.cache_way)
		sa_cache[set].pop_back();

	sa_cache[set].insert(sa_cache[set].begin(), address);
}

bool Cache::Access(uint64_t address) {
	int set = FindSet(address);
	int way = FindWay(address, set);

	if (way == -1)
		return false;
	else {
		// LRU policy
		sa_cache[set].erase(sa_cache[set].begin() + way);
		sa_cache[set].insert(sa_cache[set].begin(), address);
		return true;
	}
}

int Cache::FindSet(uint64_t address) {
	uint64_t addr_normalized = address - arch_info.xw_ele_addr_start;

	return (addr_normalized / CACHE_LINE_BYTE) % arch_info.cache_set;
}

int Cache::FindWay(uint64_t address, int set) {
	for (int i = 0; i < sa_cache[set].size(); i++) {
		if (sa_cache[set][i] == address)
			return i;
	}

	return -1;
}