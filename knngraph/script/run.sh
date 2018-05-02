mode="Debug"
# mode="Release"

cd ../../${mode}
cmake ../ -DCMAKE_BUILD_TYPE=${mode}
make -j4 knngraph_train 2>&1 | tee ../knngraph/script/log.txt
cd ../knngraph/script
log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi

app="knngraph_train"
gdb --args ../../${mode}/${app} --conf ../../conf/${app}.conf
