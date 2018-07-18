dataset = "audio"
base_fvecs_file = "../gqr/data/" + dataset + "/" + dataset + "_base" + ".fvecs"
base_output_file = dataset + "_base" + ".idfvecs"
query_fvecs_file = "../gqr/data/" + dataset + "/" + dataset + "_query" + ".fvecs"
query_output_file = dataset + "_query" + ".idfvecs"

# dataset = "audio"
# base_fvecs_file = "../gqr/data/" + dataset + "/" + dataset + "_base" + ".fvecs"
# base_output_file = dataset + "_base" + ".idfvecs"
# query_fvecs_file = "../gqr/data/" + dataset + "/" + dataset + "_query" + ".fvecs"
# query_output_file = dataset + "_query" + ".idfvecs"

import struct
fout = open(base_output_file, "wb")
with open(base_fvecs_file, "rb") as fin:
    itemId = 0
    while True:
        dimension = fin.read(4)
        if dimension == '':
            break
        int_dimension = struct.unpack('I', dimension)[0]
        vecs = fin.read(int_dimension * 4)
        fout.write(struct.pack('I' ,itemId))
        fout.write(dimension)
        fout.write(vecs)
        itemId += 1;

fout.close()

fout = open(query_output_file, "wb")
with open(query_fvecs_file, "rb") as fin:
    itemId = 0
    while True:
        dimension = fin.read(4)
        if dimension == '':
            break
        int_dimension = struct.unpack('I', dimension)[0]
        vecs = fin.read(int_dimension * 4)
        fout.write(struct.pack('I' ,itemId))
        fout.write(dimension)
        fout.write(vecs)
        itemId += 1;

fout.close()
