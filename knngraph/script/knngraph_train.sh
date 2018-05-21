cd ../../build
# cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake ../ -DCMAKE_BUILD_TYPE=Release
make knngraph_train -j4 2>&1 | tee ../knngraph/script/log.txt
cd ../knngraph/script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi

../../build/knngraph_train --conf ../../conf/knngraph_train.conf
