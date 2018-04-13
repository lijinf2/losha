# This points to a file, which should contains hostnames (one per line).
# E.g.,
#
# worker1
# worker2
# worker3
#
MACHINE_CFG=/data/losha/conf/slaves

# This point to the directory where Husky binaries live.
# If Husky is running in a cluster, this directory should be available
# to all machines.
ROOT_DIR=/data/losha/
time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $ROOT_DIR && ls conf/ > /dev/null && ls build/ > /dev/null && cd script && ./$@"
