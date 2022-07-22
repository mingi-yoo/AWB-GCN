Slice-and-Forge GNN Simulator (GNNSim)
======================

GNNSim is a simulator for Graph Convolution Networks(GCN) using Silce-and-Forge. The memory is simulated based on DRAMsim3.

# Abstract
Slice-and-Forge is an efficient hardware accelerator for GCNs which greatly improves the effectiveness of the limited on-chip cache. Unlike vertex tiling, SnF splits the features into vertical slices instead of tiling the topology data. In addition, SnF can dynamically tune its configuration space at runtime while maintaining high performance. Over the repetitions, SnF adjusts its tile size and work distribution policy to preserve the inherent locality of the given GCN.

![snf](./img/snf.png)

# Building and running
The building requirement is same as DRAMsim3.

### Building
Clone our simulator to your directory and do make
```
git clone https://github.com/acsys-yonsei/GNNSim.git
make
```

### Running
Basically, this simulator runs using 3 options
```
./sim -i configs/(configs_name).ini -x data_info.txt -a graph_csr.txt
```
'-i' option requests config file from configs folder.
'-x' option requests basic data information text file. The format is as follows
```
weight height size
weight width size
x matrix height size
the number of zero nodes of graph
```
'-a' option requests graph file. The graph must be applied csr. The format is as follows
```
values
row_index
col_index
```
Also, you can add other options
```
-t b_v : add vt operation. set tile number to b_v
-w lac_width : adjust LAC width to lac_width
-p result.csv : write some values of result to result.csv
```
The result of simulator would be writed in results folder.