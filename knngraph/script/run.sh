hadoop dfs -rm -r /losha/output
# mode="Release"
mode="Debug"
app="knnconnected_components"

cd ../../${mode}
cmake ../ -DCMAKE_BUILD_TYPE=${mode}
make -j4 ${app} 2>&1 | tee ../knngraph/script/log.txt
cd ../knngraph/script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi


gdb --args ../../${mode}/${app} --conf ../../conf/${app}.conf 
# ../../${mode}/husky/Master --conf ../../conf/${app}-slaves.conf 

