cd ../..//build
cmake ../ -DCMAKE_BUILD_TYPE=Debug
# cmake ../ -DCMAKE_BUILD_TYPE=Release
make sample_base 2>&1 | tee ../knngraph/script/log.txt
cd ../knngraph/script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi


dataset="audio"
num_samples=20
base_file="../../data/idfvecs/${dataset}/${dataset}_base.idfvecs"
sample_file="../data/${dataset}/${dataset}_sample.idfvecs"

../../build/sample_base $base_file $num_samples $sample_file

