cd ../build 
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make sample_queries 2>&1 | tee ../script/log.txt
cd ../script
log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi

num_queries=1000
dimension=300

base_file="./output/glove2.2m_base.fvecs"
query_file="./output/glove2.2m_query.fvecs"

../build/bin/sample_queries $base_file $num_queries $query_file $dimension
