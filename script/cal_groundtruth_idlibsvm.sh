mkdir tmp
g++ --std=c++11 -lpthread -I ../gqr -I ../gqr/include  -I../ -I../include -g src/cal_groundtruth_idlibsvm.cpp -o tmp/cal_groundtruth_idlibsvm 2>&1 | tee log.txt

numThreads=4;
metric="angular"
queryType="topk:20"
# queryType="radius:0.9"

iter=0
for dataset in "tweet"
do
    iter=`expr $iter + 1`

    # input data files
    base_file="../data/idlibsvm/${dataset}/${dataset}_base.idlibsvm"
    query_file="../data/idlibsvm/${dataset}/${dataset}_query.idlibsvm"

    # output groundtruth files, support both lshbox formats 
    lshbox_bench_file="../data/idlibsvm/${dataset}"

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


    tmp/cal_groundtruth_idlibsvm $base_file $query_file $queryType $lshbox_bench_file $metric $numThreads
done
