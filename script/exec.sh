# This points to a file, which should contains hostnames (one per line).
# E.g.,
#
# worker1
# worker2
# worker3
#
cd ../
ROOT_DIR=`pwd`
MACHINE_CFG="$ROOT_DIR/conf/slaves"
cd script

echo $ROOT_DIR
echo $MACHINE_CFG
# This point to the directory where Husky binaries live.
# If Husky is running in a cluster, this directory should be available
# to all machines.
time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $ROOT_DIR && ls conf/ > /dev/null && ls build/ > /dev/null && cd script && ./$@"
