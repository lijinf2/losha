hadoop dfs -rm -r /losha/output
app="knngraph_train"
# mode="Debug"
mode="Release"
# mode="RelWithDebInfo"

mkdir ../${mode}
cd ../${mode}
cmake ../ -DCMAKE_BUILD_TYPE=${mode}
make -j4 ${app} 2>&1 | tee ../script/log.txt
cd ../script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi
# ../${mode}/${app} --conf ../conf/${app}.conf
./exec.sh ../${mode}/${app} --conf ../conf/${app}-slaves.conf
