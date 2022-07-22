#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// JSON Parser
#include "json/json.h"

#include "common.h"
#include "preprocessor.h"
#include "controller.h"

using namespace std;

extern ArchInfo arch_info;
extern DataInfo data_info;
extern LogInfo log_info;

extern vector<string> split(const string& str, const string& delim);
bool exists (string& name);

void GetOption(int opt, F_PATH &f_path);

int main(int argc, char** argv) {
	F_PATH f_path;
	f_path.csv_path = "";
	f_path.davc_path = "";

	arch_info.lac_width = 32;
	arch_info.x_unit = 32;

	arch_info.mode = X_CMP;
	arch_info.gnn_mode = GCN;

	data_info.n_tiles = 1;

	int opt = 0;
	while ((opt = getopt(argc, argv, "i:x:a:t:c:m:l:u:g:d:")) != EOF)
		GetOption(opt, f_path);

	int idx = 0;
	struct stat st = {0};
	string results_path = "results";
	string result_path = results_path + "/result_" + to_string(idx);

	if (stat(results_path.c_str(), &st) == -1)
		mkdir(results_path.c_str(), 0700);

	while (stat(result_path.c_str(), &st) != -1) {
		idx++;
		result_path = results_path + "/result_" + to_string(idx);
	}

	mkdir(result_path.c_str(), 0700);
	arch_info.mem_output = result_path;

	Preprocessor pp(f_path);
	pp.PrintStatus();
	Controller* ctr = new Controller();
	uint64_t cycle = ctr->GCNInference(arch_info.mode);
	//uint64_t cycle = ctr->Combination();
	cout<<endl;
	cout<<"GCN END. TOTAL CYCLE: "<<cycle<<endl;

	// log cycle
	log_info.cycle = cycle;
	// log lac width
	log_info.lac_width = arch_info.lac_width;

	ctr->PrintMemoryStats();

	/*
	 * Save log as .csv output
	 */
	if(f_path.csv_path != "") {
		Json::Value root;
		Json::Reader reader;
		ifstream json(arch_info.mem_output + "/dramsim3.json", ifstream::binary);
		reader.parse(json, root);
		log_info.bandwidth = root["0"]["average_bandwidth"].asFloat();
		log_info.energy = root["0"]["total_energy"].asDouble();
		// Locking Mechanism for Large Scale Queueing
		string lock_csv_path = f_path.csv_path + ".lock";
		bool authorized = false;
		while(!authorized) {
			if (!exists(lock_csv_path)) {
				ofstream lockfile (lock_csv_path);
				lockfile << "Currently Locked !!!" << endl;
				lockfile.close();
				authorized = true;
			}
		}
		if(authorized) {
			// Append for CSV
			ofstream csv_out(f_path.csv_path, ios::app);
			csv_out << endl << log_info.engine_n << "," << log_info.mode << "," << log_info.mem_type << "," << log_info.cache_size << "," << log_info.tiling_info << ",";
			if (log_info.mode == 0) {
				csv_out << log_info.x_unit << ",";
			} else {
				csv_out << "-1,";
			}
			csv_out << log_info.dataset << "," << log_info.lac_width << ",";
			csv_out << log_info.feature_length << ",";
			csv_out << log_info.urb << ",";
			csv_out << log_info.bf << ",";
			csv_out << log_info.cycle << ",";
			csv_out << (log_info.g_access + log_info.f_ele_access + log_info.o_access) << ",";
			csv_out << log_info.g_access << ",";
			csv_out << log_info.f_frag_access << ",";
			csv_out << (log_info.f_ele_access - log_info.f_frag_access) << ",";
			csv_out << log_info.o_access << ",";
			csv_out << log_info.bandwidth << ",";
			csv_out << log_info.energy << ",";
			csv_out.close();
			remove(lock_csv_path.c_str()); // Remove Lockfile
			authorized = false;
		}
	}

	delete ctr;
	return 0;
}

void GetOption(int opt, F_PATH &f_path) {
	vector<string> result_v;
	string file_name;
	switch (opt) {
		case 'i':
			f_path.ini = optarg;
			break;
		case 'x':
			f_path.xw_data = optarg;
			break;
		case 'a':
			f_path.a_data = optarg;
			// log info
			result_v = split(optarg, "/");
			file_name = (string)result_v.back();
			result_v = split(file_name, ".");
			log_info.dataset = result_v.front();
			break;
		case 't':
			data_info.n_tiles = stoi(optarg);
			break;
		case 'c':
			f_path.csv_path = optarg;
			break;
		case 'm':
			arch_info.mode = (F_MODE)stoi(optarg);
			log_info.mode = arch_info.mode;
			break;
		case 'l':
			arch_info.lac_width = stoi(optarg);
			break;
		case 'u':
			arch_info.x_unit = stoi(optarg);
			log_info.x_unit = arch_info.x_unit;
			break;
		case 'g':
			arch_info.gnn_mode = (GNN_MODE)stoi(optarg);
			break;
		case 'd':
			f_path.davc_path = optarg;
			break;
		case '?':
			if (optopt == 't' || optopt == 'c' || optopt == 'm' || optopt == 'l' || optopt == 'u' || optopt == 'g' || optopt == 'd')
				break;
			else {
				cout<<"You must follow this form: \'./sim -i ini_path -x xw_data_path -a a_data_path {-t|-c|-m|-l|-u} {additional}\'" <<endl;
				exit(1);
			}
	}
}


// Function of checking file existance
bool exists (string& name) {
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}