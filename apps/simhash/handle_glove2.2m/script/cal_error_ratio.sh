#mkdir simhash_output
#cd ./simhash_output
#simhash_result_path="/ysong/output/part-*"
#hadoop fs -get $simhash_result_path ./
#cd ..

cd ../build 
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make cal_error_ratio 2>&1 | tee ../script/log.txt
cd ../script
#log=`grep error log.txt`
#if [ "$log" != "" ]; then
#    exit
#fi

thread_num=120
simhash_output="./simhash_output/part-"
groundtruth="./output/glove2.2m_groundtruth.lshbox"
output_file="error_ratio.txt"
        
../build/bin/cal_error_ratio $thread_num $simhash_output $groundtruth $output_file 
