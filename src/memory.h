#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <src/dramsim3.h>
#include "common.h"

using namespace std;

struct M_STAT {
	uint64_t g_access;
	uint64_t f_frag_access;
	uint64_t f_ele_access;
	uint64_t o_access;
};

class Memory {
public:
	M_STAT ms;
	
	Memory(const string& config_file, const string& output_dir);
	~Memory();
	void UpdateCycle();
	void AddTransaction(Transaction t);
	bool WillAcceptTransaction();
	void ConsumeTransaction();
	void PrintStats();
	R_TYPE GetGraph(int id);
	uint64_t GetFeature();

private:
	dramsim3::MemorySystem* mem;
	vector<queue<R_TYPE>> rq;
	queue<uint64_t> fq;
	queue<Transaction> tq;

	void ReadCompleteCallback(uint64_t address);
	void WriteCompleteCallback(uint64_t address);
};

#endif