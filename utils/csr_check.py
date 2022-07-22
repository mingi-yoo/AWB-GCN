import sys
if __name__ == '__main__':
    file_name = sys.argv[1]
    f = open(file_name, 'r')
    
    #val = f.readline()
    #val = val.split()
    #print("check value")
    #print(len(val))
    v = f.readline()
    v = v.split()
    print("check vertex")
    print(v[0])
    print(len(v))
    max_edge = v[len(v)-1]
    e = f.readline()
    e = e.split()
    print(max_edge)
    print(len(e))
    a = -1
    prev = 0
    zero_num = 0
    isproblem = False
    for i in v:
        cur = int(i)
        interval = cur - prev
        if interval == 0:
            zero_num = zero_num + 1
        for j in range(prev, cur-1):
            e_1 = int(e[j])
            e_2 = int(e[j+1])
            if e_1 > e_2:
                print("ERROR! " + e[j] + " must be back of " + e[j+1])
                sys.exit()
        prev = cur
    print("zero_num: " + str(zero_num))
