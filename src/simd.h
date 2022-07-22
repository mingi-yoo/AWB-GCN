#ifndef __SIMD_H__
#define __SIMD_H__

#include "common.h"
#include "memory.h"

using namespace std;

struct SIMD_STAT {
	uint64_t axw_addr_start;
	int nonzero_row;
	int cur_row_idx;
	int cur_v_fold;

	int cur_bf;
	int write_urb;

	int cnt;

	bool simd_end;
};

class SIMD {
public:
	SIMD_STAT ss;
	SIMD(int id, Memory* mem);
	~SIMD();

	void GetFeature(int f);
	void Write();
	void Print();
private:
	int id;
	Memory* mem;
	queue<uint64_t> wq;
	
	uint64_t GetAddress();
};

#endif