# This points to a file, which should contains hostnames (one per line).
# E.g.,
#
# worker1
# worker2
# worker3
#

# This point to the directory where Husky binaries live.
# If Husky is running in a cluster, this directory should be available
# to all machines.
BIN_DIR="$LOSHA_HOME/build"
CONF_DIR="$LOSHA_HOME/conf"
MACHINE_CFG="$CONF_DIR/slaves"
echo time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $CONF_DIR && ls -al > /dev/null && cd $BIN_DIR && ls -al > /dev/null && ./$@"
time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $CONF_DIR && ls -al > /dev/null && cd $BIN_DIR && ls -al > /dev/null && ./$@"

#time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "ulimit -c unlimited && cd /tmp && ./$1 $2"
