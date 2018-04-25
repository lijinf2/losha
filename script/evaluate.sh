# only supports lines of triplets in the format of (srcId, dstId, dist)

mkdir tmp
hadoop dfs -cat /losha/output/* > tmp/output.txt
# lshbox_file="../data/idfvecs/audio/audio_groundtruth.lshbox"
lshbox_file="../gqr/data/audio/audio_groundtruth.lshbox"
triplets_file="tmp/output.txt"

cd ../build 
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make evaluate_triplets ${format} 2>&1 | tee ../script/log.txt
cd ../script
log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi

../build/evaluate_triplets $lshbox_file $triplets_file
