mkdir tmp
g++ --std=c++11 -lpthread -I ../gqr -I ../gqr/include  -I../ -I../include -O3 src/cal_groundtruth_idlibsvm.cpp -o tmp/cal_groundtruth_idlibsvm 2>&1 | tee log.txt

topk=20
numThreads=20;
metric="angular"

iter=0
for dataset in "tweet"
do
    iter=`expr $iter + 1`

    # input data files
    base_file="../data/idlibsvm/${dataset}/${dataset}_base.idlibsvm"
    query_file="../data/idlibsvm/${dataset}/${dataset}_query.idlibsvm"

    # output groundtruth files, support both ivecs and lshbox formats 
    lshbox_bench_file="../data/idlibsvm/${dataset}/${dataset}_groundtruth.lshbox"
    if [ $metric != "euclidean" ]
    then
        lshbox_bench_file="../data/idlibsvm/${dataset}/${dataset}_${metric}_groundtruth.lshbox"
    fi

    tmp/cal_groundtruth_idlibsvm $base_file $query_file $topk $lshbox_bench_file $metric $numThreads
done
