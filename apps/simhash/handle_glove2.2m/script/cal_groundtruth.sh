cd ../build 
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make cal_groundtruth 2>&1 | tee ../script/log.txt
cd ../script
log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi

topk=10
dimension=300;

base_file="./output/glove2.2m_base.fvecs"
query_file="./output/glove2.2m_query.fvecs"
ivecs_bench_file="./output/glove2.2m_groundtruth.ivecs"
lshbox_bench_file="./output/glove2.2m_groundtruth.lshbox"
        
../build/bin/cal_groundtruth $base_file $query_file $topk $lshbox_bench_file $ivecs_bench_file $dimension
