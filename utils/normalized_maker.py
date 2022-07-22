import numpy as np
from scipy.sparse import csr_matrix
import torch
from torch_geometric.data import Data
from torch_sparse import SparseTensor, matmul, fill_diag, sum, mul
from torch_geometric.utils import add_remaining_self_loops
from torch_geometric.utils.num_nodes import maybe_num_nodes
from torch_geometric.utils.convert import to_scipy_sparse_matrix
from torch_scatter import scatter_add
import argparse

def gcn_norm(edge_index, edge_weight=None, num_nodes=None, improved=False,
             add_self_loops=True, dtype=None):

    fill_value = 2. if improved else 1.

    if isinstance(edge_index, SparseTensor):
        adj_t = edge_index
        if not adj_t.has_value():
            adj_t = adj_t.fill_value(1., dtype=dtype)
        if add_self_loops:
            adj_t = fill_diag(adj_t, fill_value)
        deg = sum(adj_t, dim=1)
        deg_inv_sqrt = deg.pow_(-0.5)
        deg_inv_sqrt.masked_fill_(deg_inv_sqrt == float('inf'), 0.)
        adj_t = mul(adj_t, deg_inv_sqrt.view(-1, 1))
        adj_t = mul(adj_t, deg_inv_sqrt.view(1, -1))
        return adj_t

    else:
        num_nodes = maybe_num_nodes(edge_index, num_nodes)

        if edge_weight is None:
            edge_weight = torch.ones((edge_index.size(1), ), dtype=dtype,
                                     device=edge_index.device)

        if add_self_loops:
            edge_index, tmp_edge_weight = add_remaining_self_loops(
                edge_index, edge_weight, fill_value, num_nodes)
            assert tmp_edge_weight is not None
            edge_weight = tmp_edge_weight

        row, col = edge_index[0], edge_index[1]
        deg = scatter_add(edge_weight, col, dim=0, dim_size=num_nodes)
        deg_inv_sqrt = deg.pow_(-0.5)
        deg_inv_sqrt.masked_fill_(deg_inv_sqrt == float('inf'), 0)
        return edge_index, deg_inv_sqrt[row] * edge_weight * deg_inv_sqrt[col]



if __name__ == "__main__":
    # Argument Parsing
    parser = argparse.ArgumentParser("Normalized Matrix Maker... By. JY")
    parser.add_argument('-adjm', required=True, help='adjacency matrix path')
    parser.add_argument('-normm', required=True, help='normalized matrix (result) path')
    parser.add_argument('-type', required=False, default='GCN', help='GNN Type (GCN, GAT, ...)')
    args = parser.parse_args()
    # Get Arguments
    adj_path = args.adjm
    norm_path = args.normm
    nn_type = args.type

    # Adj Matrix CSR
    # 1st line: ROW
    # 2nd line: COL
    adj_m = open(adj_path, 'r')
    adj_row = np.array(adj_m.readline().split())
    adj_col = np.array(adj_m.readline().split())
    # COL len is same as VAL len
    # + We assume that all VAL is 1
    adj_val = np.ones(shape=(len(adj_col),))
    adj_m.close()
    
    # Scipy CSR Example
    # indptr = np.array([0, 2, 3, 6])
    # indices = np.array([0, 2, 2, 0, 1, 2])
    # data = np.array([1, 2, 3, 4, 5, 6])
    # csr_matrix((data, indices, indptr), shape=(3, 3)).toarray()
    # array([[1, 0, 2],
    #     [0, 0, 3],
    #     [4, 5, 6]])

    # Acsr = csr_matrix([[1, 2, 0], [0, 0, 3], [4, 0, 5]])
    # print('Acsr',Acsr)

    # Acoo = Acsr.tocoo()
    # print('Acoo',Acoo)

    # Apt = torch.sparse.LongTensor(torch.LongTensor([Acoo.row.tolist(), Acoo.col.tolist()]),
    #                             torch.LongTensor(Acoo.data.astype(np.int32)))
    # print('Apt',Apt)

    # Adj Matrix Shape: (len(adj_row)-1, len(adj_row)-1)
    row_num = len(adj_row)-1
    adj_csr = csr_matrix((adj_val, adj_col, adj_row), shape=(row_num, row_num))
    # print(adj_csr)
    # If Error Occurs, Check CSR Format....

    adj_coo = adj_csr.tocoo()
    # print(adj_coo)
    # print(adj_coo.row.tolist())
    # print(adj_coo.col.tolist())
    # print(adj_coo.data)

    edge_index = torch.tensor([adj_coo.row.tolist(),
                                adj_coo.col.tolist()], dtype=torch.long)
    adj_G = Data(x=adj_coo.data, edge_index=edge_index) 
    # print(adj_G)

    # Normalization....
    norm_row = None
    norm_col = None
    norm_val = None
    if nn_type == 'GCN':
        norm_m = gcn_norm(adj_G.edge_index)
        norm_edge_index = norm_m[0]
        norm_data = norm_m[1]
        norm_G = to_scipy_sparse_matrix(norm_edge_index, norm_data)
        norm_G = norm_G.tocsr()
        norm_row = norm_G.indptr
        norm_col = norm_G.indices
        norm_val = norm_G.data

    # Save as CSR Format
    # 1st line: VAL
    # 2nd line: ROW
    # 3rd line: COL
    # print(norm_val)
    # print(norm_row)
    # print(norm_col)
    
    writer = open(norm_path, 'w')
    for val in norm_val:
        writer.write(str(val) + " ")
    writer.write("\n")
    for row in norm_row:
        writer.write(str(row) + " ")
    writer.write("\n")
    for col in norm_col:
        writer.write(str(col) + " ")
    writer.write("\n")
    writer.close()

    
