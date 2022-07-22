
#include "f_reader.h"

using namespace std;

extern ArchInfo arch_info;
extern DataInfo data_info;
extern LogInfo log_info;
extern vector<DataIndex> data_index;

extern vector<vector<int>> row_ptr;
extern vector<vector<int>> col_idx;

extern vector<vector<uint64_t>> x_to_addr;

extern set<int> davc_list;

extern int total_write_blk;

int f_pass;

FeatureReader::FeatureReader(Memory* mem, Cache* cah) {
	this->mem = mem;
	this->cah = cah;

	mode = X_CMP;

	// initialize status variables
	for (int i = 0; i < arch_info.n_of_engine; i++) {
		FR_STAT stat;

		stat.cur_f.is_last = false;
		stat.cur_f.dst = -1;
		stat.cur_f.v_fold = 0;
		stat.cur_f.cur_col_idx = 0;

		stat.pq_read_need = true;
		stat.on_off = false;

		frs.push_back(stat);

		// pq is pending queue : they need to request feature
		pq.push_back(queue<EdgeInfo> ());
		// tbp is to be passed queue : they check one more: is really can pass?
		fq.push_back(queue<uint64_t> ());
	}

	f_pass_end = false;

	// they need for debug; if you need, use this
	f_pass_cnt = 0;
	p_get_cnt = 0;
}

FeatureReader::~FeatureReader() {}

void FeatureReader::ModeChange(F_MODE mode) {
	// change gnn process mode
	// 0: compressed
	this->mode = mode;

	if (mode == X_CMP)
		cout<<"DO X Compressed MODE"<<endl;
	else if (mode == MAT)
		cout<<"DO MAT (Original SnF) MODE"<<endl;
	else if (mode == CSR)
		cout<<"DO X NAIVE CSR MODE"<<endl;
	else if (mode == X_FULL_CMP)
		cout<<"DO X Fully Compressed MODE"<<endl;
}

void FeatureReader::EnterFtoList(EdgeInfo ei) {
	pq[ei.id].push(ei);
}

void FeatureReader::ConsumePendingQ(int id) {
	// status update to next edge info
	EdgeInfo ei = pq[id].front();
	pq[id].pop();

	frs[id].cur_f.is_last = ei.is_last;
	frs[id].cur_f.dst = ei.dst;
	frs[id].cur_f.v_fold = ei.v_fold;
	frs[id].cur_f.cur_col_idx = 0;

	frs[id].pq_read_need = false;

	// cout<<"ID: "<<id<<endl;
	f_pass++;

	if (f_pass == data_info.num_e * arch_info.bf) {
		cout<<"ALL OF EDGE COMPLETE: "<<f_pass<<endl;
		for (int i = 0; i < arch_info.n_of_engine; i++)
			cout<<pq[i].size()<<" ";
		cout<<endl;
	}
		
}

void FeatureReader::ReadNext(int id) {
	if (fq[id].size() >= 256 || !mem->WillAcceptTransaction())
		return;

	if (frs[id].pq_read_need && !pq[id].empty())
		ConsumePendingQ(id);
	else if (frs[id].pq_read_need && pq[id].empty())
		return;

	// pass vector has 3 elements
	// pass = {current col index, (feature address to get|current col index if feature alread exist), is_last}
	if (mode == X_CMP) {
		uint64_t f_addr = ReturnAddress(frs[id].cur_f.dst, frs[id].cur_f.cur_col_idx, frs[id].cur_f.v_fold, 0);

		if (!cah->Access(f_addr) && (requested.find(f_addr) == requested.end())) {
			requested.insert(f_addr);
			mem->AddTransaction({f_addr, READ});
		}

		if (cah->Access(f_addr))
			fq[id].push(1);
		else
			fq[id].push(f_addr);
		
		int cur_urb = frs[id].cur_f.v_fold * arch_info.urb + frs[id].cur_f.cur_col_idx;
		// if (cur_urb > 16)
		//  	cout<<"ID: "<<id<<", pq size: "<<pq[id].size()<<", fq size: "<<fq[id].size()<<endl;

		if ((cur_urb != data_info.total_urb -1) && (frs[id].cur_f.cur_col_idx != arch_info.urb - 1)) {
			frs[id].cur_f.cur_col_idx++;
			uint64_t next_f_addr = ReturnAddress(frs[id].cur_f.dst, frs[id].cur_f.cur_col_idx, frs[id].cur_f.v_fold, 0);
			if (next_f_addr == 0) {
				frs[id].pq_read_need = true;
				frs[id].cur_f.cur_col_idx = arch_info.urb - 1; 
				if (frs[id].cur_f.is_last)
					fq[id].push(0);	
			}
		}
		else {
			frs[id].pq_read_need = true;
			if (frs[id].cur_f.is_last)
				fq[id].push(0);	
		}
			
	}
	else if (mode == MAT) {
		uint64_t f_addr = ReturnAddress(frs[id].cur_f.dst, frs[id].cur_f.cur_col_idx, frs[id].cur_f.v_fold, 0);
		if (!cah->Access(f_addr) && (requested.find(f_addr) == requested.end())) {
			requested.insert(f_addr);
			mem->AddTransaction({f_addr, READ});
		}
		fq[id].push(f_addr);

		int cur_urb = frs[id].cur_f.v_fold * arch_info.urb + frs[id].cur_f.cur_col_idx;
		if ((cur_urb != data_info.total_urb - 1) && (frs[id].cur_f.cur_col_idx != arch_info.urb - 1))
			frs[id].cur_f.cur_col_idx++;
		else {
			frs[id].pq_read_need = true;
			if (frs[id].cur_f.is_last)
				fq[id].push(0);	
		}
	}
	else if (mode == CSR) {
		if (frs[id].cur_f.cur_col_idx == 0) {
			uint64_t f_addr = x_to_addr[frs[id].cur_f.dst][0];
			if (!cah->Access(f_addr)) {
				if (requested.find(f_addr) == requested.end()) {
					requested.insert(f_addr);
					mem->AddTransaction({f_addr, READ});
				}
				return;
			}
			else
				frs[id].cur_f.cur_col_idx++;
		}
		uint64_t f_addr = x_to_addr[frs[id].cur_f.dst][frs[id].cur_f.cur_col_idx];
		if (!cah->Access(f_addr) && (requested.find(f_addr) == requested.end())) {
			requested.insert(f_addr);
			mem->AddTransaction({f_addr, READ});
		}
		fq[id].push(f_addr);

		if (frs[id].cur_f.cur_col_idx != x_to_addr[frs[id].cur_f.dst].size() - 1) {
			frs[id].cur_f.cur_col_idx++;
		}
		else {
			frs[id].pq_read_need = true;
			if (frs[id].cur_f.is_last)
				fq[id].push(0);
		}
	}
	else if (mode == X_FULL_CMP) {
		if (frs[id].cur_f.cur_col_idx == 0) {
			uint64_t f_addr = x_to_addr[frs[id].cur_f.dst][0];
			if (!cah->Access(f_addr)) {
				if (requested.find(f_addr) == requested.end()) {
					requested.insert(f_addr);
					mem->AddTransaction({f_addr, READ});
				}
				return;
			}
			else
				frs[id].cur_f.cur_col_idx++;
		}
		uint64_t f_addr = x_to_addr[frs[id].cur_f.dst][frs[id].cur_f.cur_col_idx];
		if (!cah->Access(f_addr) && (requested.find(f_addr) == requested.end())) {
			requested.insert(f_addr);
			mem->AddTransaction({f_addr, READ});
		}
		fq[id].push(f_addr);
		if (frs[id].cur_f.cur_col_idx != x_to_addr[frs[id].cur_f.dst].size() - 1) {
			frs[id].cur_f.cur_col_idx++;
		}
		else {
			frs[id].pq_read_need = true;
			if (frs[id].cur_f.is_last)
				fq[id].push(0);
		}
	}
}

void FeatureReader::FeatureReceive() {
	while (true){
		// get feature from memory
		// if there is no return, memory return max value
		uint64_t address = mem->GetFeature();

		if (address == numeric_limits<uint64_t>::max())
			break;

		// insert cache and remove from request list
		cah->Insert(address);
		requested.erase(address);
		f_pass_cnt++;
		//cout<<"GET: "<<f_pass_cnt<<endl;
	}
}

int FeatureReader::PassFtoSIMD(int id) { 
	// if there is no pass data or cannot pass
	// return -1
	if (fq[id].empty())
		return -1;

	uint64_t next = fq[id].front();
	//cout<<"ID: "<<id<<", NEXT: "<<next<<endl;
	
	if (next == 1 || cah->Access(next)) {
		fq[id].pop();
	}
	else {
		if (requested.find(next) == requested.end()) {
			requested.insert(next);
			mem->AddTransaction({next, READ});
		}
		return -1;
	}

	if (!fq[id].empty() && fq[id].front() == 0) {
		fq[id].pop();
		return 0;
	}
	else
		return 1;
		
}

uint64_t FeatureReader::ReturnAddress(int row, int col, int fold, bool bi_csr) {
	int dst = fold * arch_info.urb + col;
	return x_to_addr[row][dst];
}