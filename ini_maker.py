import os
import sys
import argparse
import subprocess

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="SGCN Config Generator")
    parser.add_argument('-c', required=True, help="cache size except unit")
    parser.add_argument('-u', required=True, help="cache size unit")
    parser.add_argument('-n', required=True, help="number of engines")
    args = parser.parse_args()


    config_dir_path = f"configs/{args.c}{args.u}"

    try:
        if not os.path.exists(config_dir_path):
            os.makedirs(config_dir_path)
    except OSError:
        print("directory making error!")
    
    os.chdir(config_dir_path)

    bf_list = ['1', '2', '4', '8', '16']

    for bf in bf_list:
        config = open(f"bf{bf}_{args.n}.ini", 'w')

        config.write(f"CacheSize = {args.c}\n")
        config.write(f"UnitSize = {args.u}\n")
        config.write("CacheWay = 16\n\n")
        config.write(f"NumberOfEngine = {args.n}\n\n")
        config.write(f"BF = {bf}\n\n")
        config.write(f"MemoryType = DRAMsim3/configs/HBM2_8Gb_x128.ini\n")

        config.close()
    



