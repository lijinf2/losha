#fvecs_file = "../gqr/data/audio/audio_base.fvecs" 
#output_file = "./audio_base.idfvecs"

dataset = "audio_base"
fvecs_file = "../gqr/data/audio/" + dataset + ".fvecs"
output_file = "tmp/" + dataset + ".idfvecs"

import struct
fout = open(output_file, "wb")
with open(fvecs_file, "rb") as fin:
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
