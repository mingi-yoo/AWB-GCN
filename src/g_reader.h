#ifndef __G_READER_H__
#define __G_READER_H__

#include "common.h"
#include "memory.h"

using namespace std;

struct GR_STAT {
	int row_end_idx;
	int col_end_idx;
	int val_end_idx;

	int cur_row_idx;
	int cur_col_idx;
	int cur_val_idx;

	int row_overflow;
	int col_overflow;
	int val_overflow;

	int remain_edge;

	uint64_t next_v_addr;
	uint64_t next_e_addr;
	uint64_t next_val_addr;
};

struct GR_FLAG {
	bool v_req_over;
	bool e_req_over;

	bool v_reci_over;
	bool e_reci_over;
	bool val_reci_over;

	bool reci_over;

	bool pass_over;
};

class GraphReader {
public:
	int id;
	GR_STAT grs;
	GR_FLAG grf;

	GraphReader(int id, Memory* mem);
	~GraphReader();

	// Request function
	void VertexRequest();
	void EdgeRequest();

	// Receive function
	void GraphReceive();
	void Reset();

	// Pass feature to Global Buffer
	EdgeInfo NextFeature();

private:
	

	Memory* mem;

	// for checking graph read end
	int v_req_count;
	int e_req_count;

	int v_reci_count;
	int e_reci_count;
	int val_reci_count;

	// for checking graph pass end
	int g_pass_count;

};

#endif