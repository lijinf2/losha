cd ../..//build
cmake ../ -DCMAKE_BUILD_TYPE=Debug
# cmake ../ -DCMAKE_BUILD_TYPE=Release
make cal_groundtruth_idfvecs 2>&1 | tee ../knngraph/script/log.txt
cd ../knngraph/script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi


dataset="audio"
base_file="../../data/idfvecs/${dataset}/${dataset}_base.idfvecs"
sample_file="../data/${dataset}/${dataset}_sample.idfvecs"
lshbox_bench_file="../data/${dataset}/${dataset}_sample_groundtruth.lshbox"

numThreads=4;
queryType="topk:21"
metric="euclidean"

../../build/cal_groundtruth_idfvecs $base_file $sample_file $queryType $lshbox_bench_file $metric $numThreads
