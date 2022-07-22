import argparse
import numpy as np

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Sample Graph Generator")
    parser.add_argument("-alpha", required=False, default=0.25, type=float, help="Density of graph")
    parser.add_argument("-path", required=True, help="File Path")
    parser.add_argument("-row_size", required=False, default=64, type=int, help="row size")
    parser.add_argument("-col_size", required=False, default=64, type=int, help="col size")
    args = parser.parse_args()

    nums = np.ones(args.row_size*args.col_size)
    nums[:int(args.row_size*args.col_size*args.alpha)] = 0
    np.random.shuffle(nums)
    nums = np.reshape(nums, (args.row_size, args.col_size))
    
    f = open(args.path, 'w')
    for row in range(0, args.row_size):
        for col in range(0, args.col_size):
            if nums[row, col] == 1:
                line = str(row) + " " + str(col) + "\n"
                f.write(line)
    f.close()
