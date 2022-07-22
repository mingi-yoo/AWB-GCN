#include "g_reader.h"

using namespace std;

extern vector<vector<int>> row_ptr;
extern vector<vector<int>> col_idx;

extern ArchInfo arch_info;
extern DataInfo data_info;
extern LogInfo log_info;
extern vector<DataIndex> data_index;

GraphReader::GraphReader(int id, Memory* mem) {
	this->id = id;
	this->mem = mem;

	// initialize GR_STAT
	// end index is the last index of prefetched graph
	grs.row_end_idx = 0;
	grs.col_end_idx = 0;
	grs.val_end_idx = 0;

	// cur index is the index that should be passed next
	grs.cur_row_idx = 1;
	grs.cur_col_idx = 0;
	grs.cur_val_idx = 0;

	// if whole graph is prefetched, count up overflow value
	// so, end index can move freely.
	grs.row_overflow = 0;
	grs.col_overflow = 0;
	grs.val_overflow = 0;

	// it needs for check whether current row is end or not.
	grs.remain_edge = 0;

	// it saves next address to request
	grs.next_v_addr = data_index[id].row_addr_start;
	grs.next_e_addr = data_index[id].col_addr_start;
	grs.next_val_addr = data_index[id].value_addr_start;

	// count values are need the graph prefetch is end or not
	// if count = bf, then graph request/receive is end
	v_req_count = 0;
	e_req_count = 0;

	v_reci_count = 0;
	e_reci_count = 0;
	val_reci_count = 0;

	// it count the whole graph pass
	// it needs for v_fold
	g_pass_count = 0;

	// initialize GR_FLAG
	// they need for stop passing
	grf.v_req_over = false;
	grf.e_req_over = false;

	grf.v_reci_over = false;
	grf.e_reci_over = false;
	grf.val_reci_over = false;

	grf.reci_over = false;

	grf.pass_over = false;

	if (arch_info.gnn_mode == OTHER) {
		grs.val_overflow = arch_info.bf;
		grf.val_reci_over = true;
		grs.val_end_idx = col_idx[id].size();
	}
		
}

GraphReader::~GraphReader() {}

void GraphReader::VertexRequest() {
	if (mem->WillAcceptTransaction()) {
		// Request to memory
		mem->AddTransaction({grs.next_v_addr, READ});

		grs.next_v_addr += CACHE_LINE_BYTE;
		if (grs.next_v_addr == data_index[id].row_addr_end) {
			grf.v_req_over = true;
		}
	}
}	

void GraphReader::EdgeRequest() {
	if (mem->WillAcceptTransaction()) {
		// Request to memory
		mem->AddTransaction({grs.next_e_addr, READ});
		if (arch_info.gnn_mode == GCN) {
			mem->AddTransaction({grs.next_val_addr, READ});
			grs.next_val_addr += CACHE_LINE_BYTE;
		}
	
		grs.next_e_addr += CACHE_LINE_BYTE;
		
		// we only check the col_idx request address, because value address follows automatically
		if (grs.next_e_addr == data_index[id].col_addr_end) {
			grf.e_req_over = true;
		}
	}
}

void GraphReader::GraphReceive() {
	// receive data from memory
	// memory return what type of graph
	// there are 4 types : Row, Column, Value, Empty
	R_TYPE reci = mem->GetGraph(id);

	if (reci == Row) {
		grs.row_end_idx += CACHE_LINE_COUNT;
		if (grs.row_end_idx >= row_ptr[id].size()) {
			grf.v_reci_over = true;
		}
	}
	else if (reci == Column) {
		grs.col_end_idx += CACHE_LINE_COUNT;
		if (grs.col_end_idx >= col_idx[id].size()) {
				grf.e_reci_over = true;
		}
	}
	else if (reci == Value) {
		grs.val_end_idx += CACHE_LINE_COUNT;
		if (grs.val_end_idx >= col_idx[id].size()) {
				grf.val_reci_over = true;
		}
	}

	// if all of graph is received, then turn on the receive over flag
	if (grf.v_reci_over && grf.e_reci_over && grf.val_reci_over) {
		grf.reci_over = true;
		cout<<"tile: "<<id<<" g read over"<<endl;
	}
		
}

void GraphReader::Reset() {
	id++;
	// initialize GR_STAT
	// end index is the last index of prefetched graph
	grs.row_end_idx = 0;
	grs.col_end_idx = 0;
	grs.val_end_idx = 0;

	// cur index is the index that should be passed next
	grs.cur_row_idx = 1;
	grs.cur_col_idx = 0;
	grs.cur_val_idx = 0;

	// if whole graph is prefetched, count up overflow value
	// so, end index can move freely.
	grs.row_overflow = 0;
	grs.col_overflow = 0;
	grs.val_overflow = 0;

	// it needs for check whether current row is end or not.
	grs.remain_edge = 0;

	// it saves next address to request
	grs.next_v_addr = data_index[id].row_addr_start;
	grs.next_e_addr = data_index[id].col_addr_start;
	grs.next_val_addr = data_index[id].value_addr_start;

	// count values are need the graph prefetch is end or not
	// if count = bf, then graph request/receive is end
	v_req_count = 0;
	e_req_count = 0;

	v_reci_count = 0;
	e_reci_count = 0;
	val_reci_count = 0;

	// it count the whole graph pass
	// it needs for v_fold
	g_pass_count = 0;

	// initialize GR_FLAG
	// they need for stop passing
	grf.v_req_over = false;
	grf.e_req_over = false;

	grf.v_reci_over = false;
	grf.e_reci_over = false;
	grf.val_reci_over = false;

	grf.reci_over = false;

	grf.pass_over = false;
}

EdgeInfo GraphReader::NextFeature() {
	// pass edge information to f_reader
	// there are two cases: current index does not reach to end index
	//                      end index is back of current index but is overflowed
	if (grs.cur_col_idx == col_idx[id].size() && g_pass_count == arch_info.bf - 1) {
		grf.pass_over = true;

		return {id, false, -1, -1};
	}	
	if ((grs.cur_row_idx < grs.row_end_idx) || grs.row_overflow > 0) {
		// if vertex has no edge, then skip
		while (grs.remain_edge == 0) {
			// if row index reaches to end but overflows
			if ((grs.cur_row_idx == row_ptr[id].size()) && grs.row_overflow > 0) {
				grs.cur_row_idx = 1;
				grs.row_overflow--;
			}
			else if ((grs.cur_row_idx == row_ptr[id].size()) && grs.row_overflow == 0)
				break;

			// get number of edges and update current row index
			grs.remain_edge = row_ptr[id][grs.cur_row_idx] - row_ptr[id][grs.cur_row_idx-1];
			grs.cur_row_idx++;

			if (grs.cur_row_idx == grs.row_end_idx && grs.row_overflow == 0)
				break;
			
		}
	}
	// if current row has edge, then pass edge information
	if (grs.remain_edge > 0) {
		if (((grs.cur_col_idx < grs.col_end_idx) || grs.col_overflow > 0) &&
			((grs.cur_val_idx < grs.val_end_idx) || grs.val_overflow > 0)) {
			// if current col index reach to end
			if (grs.cur_col_idx == col_idx[id].size()) {
				// if still data remain, then back to first
				if (grs.col_overflow > 0) {
					grs.cur_col_idx = 0;
					grs.cur_val_idx = 0;
					grs.col_overflow--;
					grs.val_overflow--;

					g_pass_count++;
						
				}

				if (g_pass_count == arch_info.bf) {
					grf.pass_over = true;
					//cout<<"ID: "<<id<<" PASS. "<<g_pass_count<<endl;
					
					return {id, false, -1, -1};
				}
			}
			// EdgeInfo has 4 stat : id, end edge of current vertex, column index (dst), v_fold
			EdgeInfo ret;

			ret.id = id;

			int dst = col_idx[id][grs.cur_col_idx];
			bool is_last = false;

			grs.cur_col_idx++;
			grs.cur_val_idx++;
			grs.remain_edge--;

			// if no remain edge, mark that is end of vertex
			if (grs.remain_edge == 0) 
				is_last = true;

			ret.is_last = is_last;
			ret.dst = dst;
			ret.v_fold = g_pass_count;

			return ret;
		}
	}
	
	// if g_reader cannot pass edge, then return dummy data
	return {id, false, -1, -1};
}