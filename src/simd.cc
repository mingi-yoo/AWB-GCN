#include "simd.h"

using namespace std;

extern ArchInfo arch_info;
extern DataInfo data_info;
extern LogInfo log_info;
extern vector<DataIndex> data_index;

extern vector<vector<int>> row_ptr;
extern vector<vector<int>> col_idx;

extern int total_write_blk;
extern uint64_t total_write;


SIMD::SIMD(int id, Memory* mem) {
	this->id = id;
	this->mem = mem;

	ss.axw_addr_start = arch_info.axw_addr_start;
	ss.axw_addr_start += (uint64_t)ceil((float)data_info.a_h/arch_info.n_of_engine) * id * arch_info.bf * arch_info.urb * CACHE_LINE_BYTE;
	ss.cur_row_idx = 0;
	ss.cur_v_fold = 0;
	ss.cnt = 0;
	ss.nonzero_row = data_index[id].row - data_index[id].zero_row;

	ss.cur_bf = 1;
	ss.write_urb = arch_info.urb;

	ss.simd_end = false;
}

SIMD::~SIMD() {}

void SIMD::GetFeature(int f) {
	if (f == 0) {
		for (int i = 0; i < ss.write_urb; i++) {
			uint64_t address = GetAddress();
			wq.push(address);
			ss.cur_v_fold++;
		}
		ss.cur_v_fold = 0;
		ss.cur_row_idx++;
		if (ss.cur_row_idx == ss.nonzero_row) {
			ss.cur_row_idx = 0;
			ss.cur_bf++;
			if (ss.cur_bf == arch_info.bf)
				ss.write_urb = data_info.total_urb - (arch_info.urb * (arch_info.bf - 1));
		}
	}
}

void SIMD::Write() {
	if (wq.empty() || !mem->WillAcceptTransaction())
		return;

	uint64_t address = wq.front();
	wq.pop();
	mem->AddTransaction({address, WRITE});
}

uint64_t SIMD::GetAddress() {
	uint64_t address = ss.axw_addr_start;
	address += ss.cnt * CACHE_LINE_BYTE;
	ss.cnt++;

	return address;
}

void SIMD::Print() {
	cout<<"ID: "<<id<<", total write: "<<ss.cnt<<endl;
}