# only supports lines of triplets in the format of (srcId, dstId, dist)

# mkdir tmp
rm -rf tmp/output.txt
hadoop dfs -cat /losha/output/* > tmp/output.txt

lshbox_file="../gqr/data/audio/audio_groundtruth.lshbox"
triplets_file="tmp/output.txt"

binary_file="tmp/evaluate_triplets"
if [ ! -f $binary_file ]; then
    g++ --std=c++11 -I ../gqr -I ../gqr/include  -O333 src/evaluate_triplets.cpp -o $binary_file 2>&1 | tee log.txt
fi

tmp/evaluate_triplets $lshbox_file $triplets_file
