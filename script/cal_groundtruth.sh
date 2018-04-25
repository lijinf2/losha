numThreads=4;
dataset="audio"

metric="euclidean"
# metric="angular"

queryType="topk:20"
# queryType="radius:0.9"

# format="idlibsvm"
format="idfvecs"

cd ../build 
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make cal_groundtruth_${format} 2>&1 | tee ../script/log.txt
cd ../script


base_file="../data/${format}/${dataset}/${dataset}_base.${format}"
query_file="../data/${format}/${dataset}/${dataset}_query.${format}"

# output groundtruth files, support both lshbox formats 
lshbox_bench_file="../data/${format}/${dataset}"

if [ $metric == "euclidean" ]
then 
    lshbox_bench_file="${lshbox_bench_file}/${dataset}_groundtruth.lshbox"
else
    lshbox_bench_file="${lshbox_bench_file}/${dataset}_${metric}_groundtruth.lshbox"
fi

if [[ $queryType == "radius:"* ]]
then
    lshbox_bench_file="${lshbox_bench_file}_radius"
fi

../build/cal_groundtruth_$format $base_file $query_file $queryType $lshbox_bench_file $metric $numThreads
