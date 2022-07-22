#include "memory.h"

using namespace std;

extern ArchInfo arch_info;
extern vector<DataIndex> data_index;

extern uint64_t total_write;

extern int f_pass;
uint64_t tq_get;

uint64_t cycle;

extern uint64_t xw_get;

Memory::Memory(const string& config_file, const string& output_dir) {
	mem = dramsim3::GetMemorySystem(config_file,
									output_dir,
									std::bind(&Memory::ReadCompleteCallback, this, std::placeholders::_1),
									std::bind(&Memory::WriteCompleteCallback, this, std::placeholders::_1));

	for (int i = 0; i < arch_info.n_of_engine; i++)
		rq.push_back(queue<R_TYPE> ());
	
	ms.g_access = 0;
	ms.f_frag_access = 0;
	ms.f_ele_access = 0;
	ms.o_access = 0;
}

Memory::~Memory() {
	delete mem;
}

void Memory::UpdateCycle() {
	mem->ClockTick();
	cycle++;
}

void Memory::AddTransaction(Transaction t) {
	tq.push(t);
	tq_get++;
}

void Memory::ConsumeTransaction() {
	while (!tq.empty()) {
		Transaction t = tq.front();
		if (mem->WillAcceptTransaction(t.address, t.is_write)) {
			mem->AddTransaction(t.address, t.is_write);
			tq.pop();
			//cout<<"TQ SIZE: "<<tq.size()<<endl;
		}
		else
			break;
	}
}

bool Memory::WillAcceptTransaction() {
	if (tq.size() < REQUEST_LIMIT)
		return true;
	else
		return false;
}

void Memory::PrintStats() {
	mem->PrintStats();
}

void Memory::ReadCompleteCallback(uint64_t address) {
	uint64_t f_addr_start;

	f_addr_start = arch_info.xw_ele_addr_start;

	if (address >= arch_info.d_value_addr_start && address < f_addr_start) {
		// Graph data
		R_TYPE type;
		if (address < arch_info.a_row_addr_start)
			type = Value;
		else if (address < arch_info.a_col_addr_start)
			type = Row;
		else if (address < f_addr_start)
			type = Column;

		int id;

		for (int i = 0; i < arch_info.n_of_engine; i++) {
			if (type == Value) {
				if (address >= data_index[i].value_addr_start && address < data_index[i].value_addr_end) {
					id = i;
					break;
				}
			}
			else if (type == Row) {
				if (address >= data_index[i].row_addr_start && address < data_index[i].row_addr_end) {
					id = i;
					break;
				}
			}
			else if (type == Column) {
				if (address >= data_index[i].col_addr_start && address < data_index[i].col_addr_end) {
					id = i;
					break;
				}
			}
		}
		ms.g_access++;
		rq[id].push(type);
	}
	else if (address >= f_addr_start) {
		ms.f_ele_access++;
		xw_get++;
		//cout<<"F RETURN: "<<hex<<address<<dec<<", cycle: "<<cycle<<endl;
		//cout<<"f ele: "<<ms.f_ele_access<<", "<<hex<<address<<dec<<endl;
		fq.push(address);
	}
}

R_TYPE Memory::GetGraph(int id) {
	if (rq[id].empty())
		return Empty;
	else {
		R_TYPE ret = rq[id].front();
		rq[id].pop();
		return ret;
	}
}

uint64_t Memory::GetFeature() {
	if (fq.empty())
		return numeric_limits<uint64_t>::max();
	else {
		uint64_t ret = fq.front();
		fq.pop();
		return ret;
	}
}

void Memory::WriteCompleteCallback(uint64_t address) {
	// TO-DO
	ms.o_access++;
	total_write--;
}