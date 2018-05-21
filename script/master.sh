# mode="Debug"
mode="Debug"
app="linearscan"

cd ../${mode}
cmake ../ -DCMAKE_BUILD_TYPE=${mode}
make -j4 Master 2>&1 | tee ../script/log.txt
cd ../script

log=`grep error log.txt`
if [ "$log" != "" ]; then
    exit
fi


# ../${mode}/husky/Master --conf ../conf/${app}.conf 
../${mode}/husky/Master --conf ../conf/${app}-slaves.conf 

