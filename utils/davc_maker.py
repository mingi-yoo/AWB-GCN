import argparse

"""
DAVC Generator

argument:
    -i (input edgelist graph path) -o (davc list output path) -s (davc cache size(MB)) -t (vt == |Bv|)
e.g.
$ python3 davc_maker.py -i /nfs/home/gcnacc_data/pokec.el -o ./pokec_davc_8MB.txt -s 8 -t 4
"""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="DAVC Generator")
    parser.add_argument("-i", required=True, help="graph file path (edge list)")
    parser.add_argument("-o", required=True, help="DAVC output path")
    parser.add_argument("-s", required=True, type=int, help="DAVC cache size")
    parser.add_argument("-t", required=True, type=int, help="VT |Bv|")
    args = parser.parse_args()

    graph_path = args.i
    # Max Index Calculation
    max_idx = 0
    
    # read graph
    tot_g_dict = {}
    f = open(graph_path, 'r')
    lines = f.readlines()
    for line in lines:
        src_dst = line.split()
        src = int(src_dst[0])
        dst = int(src_dst[1])
        if dst in tot_g_dict:
            tot_g_dict[dst].append(src)
        else:
            tot_g_dict[dst] = [src]
        if src > max_idx:
            max_idx = src
        if dst > max_idx:
            max_idx = dst
    f.close()

    # for end index... increase max_idx 1
    max_idx += 1

    # vt indexing
    if args.t == 1:
        vt_partial_idx = [max_idx]
    else:
        vt_partial_idx = []
        for i in range(0, args.t):
            vt_partial_idx.append(int(max_idx/args.t))
        remaining = max_idx%args.t
        for i in range(0, remaining):
            vt_partial_idx[i] += 1
        for i in range(1, len(vt_partial_idx)):
            vt_partial_idx[i] = vt_partial_idx[i] + vt_partial_idx[i-1]
    print(">>>>> VT End Indices <<<<<")
    print(vt_partial_idx)

    vt_current_idx = [0]
    if args.t != 1:
        for i in range(0, len(vt_partial_idx)-1):
            vt_current_idx.append(vt_partial_idx[i])
    print(">>>>> VT Start Indices <<<<<")
    print(vt_current_idx)

    # make g_dict for each vt
    g_dicts = []
    for i in range(args.t):
        g_dicts.append({})
    for dst, src_list in tot_g_dict.items():
        for vt_i in range(0, args.t):
            if dst < vt_partial_idx[vt_i]:
                g_dicts[vt_i][dst] = src_list
                break
    
    # element num calculation
    element_num = args.s * 1024
    element_per_vts = []
    for i in range(0, args.t):
        element_per_vts.append(element_num)
    remaining = element_num
    
    # sorting by length of connection
    # and save in sorted_elements
    sorted_elements = []
    for vt_i in range(0, args.t):
        sorted_dict = sorted(g_dicts[vt_i], key=lambda k: len(g_dicts[vt_i][k]), reverse=True)
        for element in sorted_dict[:element_per_vts[vt_i]]:
            sorted_elements.append(element)
    print(">>>>> Total ", str(len(sorted_elements)), " are DAVC elements <<<<<")

    # write to output
    f = open(args.o, 'w')
    for element in sorted_elements:
        f.write(str(element) + " ")
    f.close()
    print(">>>>> Write output file <<<<<")
