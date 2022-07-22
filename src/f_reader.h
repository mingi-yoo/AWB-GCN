#ifndef __F_READER_H__
#define __F_READER_H__

#include "common.h"
#include "memory.h"
#include "cache.h"

using namespace std;

struct F_STAT {
	bool is_last;

	int dst;
	int v_fold;
	int cur_col_idx;
};

struct FR_STAT {
	F_STAT cur_f;

	bool pq_read_need;

	// only csr mode use
	bool on_off;
	vector<uint64_t> pass;
};

class FeatureReader {
public:
	bool f_pass_end;
	
	FeatureReader(Memory* mem, Cache* cah);
	~FeatureReader();

	void ModeChange(F_MODE mode);
	void EnterFtoList(EdgeInfo ei);
	void ReadNext(int id);
	void FeatureReceive();
	int PassFtoSIMD(int id);
private:
	Memory* mem;
	Cache* cah;

	F_MODE mode;
	vector<FR_STAT> frs;

	vector<queue<EdgeInfo>> pq;
	set<uint64_t> requested;
	
	vector<queue<uint64_t>> fq;

	int f_requested;

	int f_pass_cnt;
	int p_get_cnt;

	void ConsumePendingQ(int id);
	uint64_t ReturnAddress(int row, int col, int fold, bool bi_csr);
};

#endif