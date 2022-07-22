#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "common.h"
#include "memory.h"
#include "g_reader.h"
#include "f_reader.h"
#include "controller.h"
#include "simd.h"

using namespace std;

class Controller {
public:
	Controller();
	~Controller();

	uint64_t GCNInference(int f_mode);
	void PrintMemoryStats();
	uint64_t Combination();
	uint64_t DAVCRead();
private:
	Memory* mem;
	Cache* cah;
	GraphReader* gr;
	SIMD* si;
	FeatureReader* fr;
};

#endif