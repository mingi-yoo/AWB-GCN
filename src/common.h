#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <utility>
#include <ctime>
#include <limits>

// for transaction
#define WRITE true
#define READ false

#define CACHE_LINE_COUNT 16
#define CACHE_LINE_BYTE 64
#define CACHE_LINE_BIT 512

#define REQUEST_LIMIT 10240

using namespace std;

// define enumerated types
enum R_TYPE {Row, Column, Value, Empty};
enum F_MODE {X_CMP, MAT, CSR, X_FULL_CMP};
enum GNN_MODE {GCN, OTHER};

struct F_PATH {
	string ini;
	string a_data;
	string xw_data;
	string csv_path;
	string davc_path;
};

struct ArchInfo {
	int n_of_engine;

	int cache_size;
	int cache_way;
	int cache_set;
	int output_limit;

	int urb; // urb = w_w / bf / cache_line_count
	int bf; 
	int lac_width;

	int x_unit; //only use in 
	F_MODE mode;
	GNN_MODE gnn_mode;

	uint64_t d_value_addr_start;
	uint64_t a_row_addr_start;
	uint64_t a_col_addr_start;

	// use X_CMP or MAT mode
	uint64_t xw_ele_addr_start;
	// use CSR mode
	uint64_t xw_value_addr_start;
	uint64_t xw_row_addr_start;
	uint64_t xw_col_addr_start;

	uint64_t axw_addr_start;

	string mem_type;
	string mem_output;
};

struct LogInfo {
	int mode; // mode 0: x compress, mode 1: SnF
	int x_unit;

	int engine_n; // (done) (done)
	string mem_type; // (done) (done)
	int cache_size; // (done) (done)
	// int output_buffer_size; // ini
	string dataset; // (done) (done)
	
	// bool is_fine; // ini
	int lac_width; // (done) (done)
	int feature_length; // (done) (done)
	int urb; // (done) (done)
	int bf; // (done) (done)
	string tiling_info; // (done) (done)
	uint64_t cycle; // (done) (done)
	uint64_t f_frag_access; // (done) (done)
	uint64_t f_ele_access; // (done) (done)
	uint64_t o_access; // (done) (done)
	uint64_t g_access; // (done) (done)


	float bandwidth; // json parsing 
	double energy; // json parsing
	
	// uint64_t total_write; // result print
};

struct DataInfo {
	int a_h;
	int a_w;
	int x_h;
	int x_w;
	int w_h;
	int w_w;

	int n_tiles;

	int total_urb;

	int num_e;
	int num_v;

	int num_frag;
};

struct DataIndex {
	uint64_t value_addr_start;
	uint64_t value_addr_end;
	uint64_t row_addr_start;
	uint64_t row_addr_end;
	uint64_t col_addr_start;
	uint64_t col_addr_end;

	int row;
	int zero_row;
	int total_write;
};

struct EdgeInfo {
	int id;

	bool is_last;
	int dst;
	int v_fold;
};

struct Transaction {
	uint64_t address;
	bool is_write;
};

#endif
