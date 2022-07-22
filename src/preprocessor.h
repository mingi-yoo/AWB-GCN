#ifndef __PREPROCESSOR_H__
#define __PREPROCESSOR_H__

#include "common.h"

using namespace std;

class Preprocessor {
public:
	Preprocessor(F_PATH &f_path);
	~Preprocessor();

	void PrintStatus();
private:
	vector<vector<int>> xw;
	map<string, string> m_table;
	void ReadIni(string ini);
	void ReadData(string a_data, string xw_data);
	void ReadDAVC(string davc_path);
	void LAC();
	void Tiling();
	void TransXW();
	void AddressMapping();

	void ParseIni(string csv_path);
	bool Contain(string name);
	string GetString(string name);
	int GetInt(string name);
	uint64_t GetUint64(string name);
};

#endif