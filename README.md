# Losha Losha is a framework focusing on Distributed Similarity Search

## Apps
  - simhash
  - e2lsh

## Build
Download the source code of losha:

    git clone --recursive https://github.com/lijinf2/losha.git

We assume the root directory of Losha is `$Losha`.

    $ cd $Losha
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..  
    $ make help                      # List all build target
    $ make -j{N} Master
    $ make -j{N} $ApplicationName    # Build any Losha application

## Configuration
Configuration files should be provided for different applications to fit the user's specific requirments and parameter for nearest neighbor search. An example file for configuration is provided as follows:

    # Required
    master_host=xxx.xxx.xxx.xxx
    master_port=yyyyy
    comm_port=yyyyy

    # Data management
    outputPath=zzzz
    itemPath=zzzz
    queryPath=zzzz

    # Optional Cluster Config
    hdfs_namenode=xxx.xxx.xxx.xxx
    hdfs_namenode_port=yyyyy

    # For Master
    serve=1
    hostname=localhost
    port=2016

    # Session for worker information
    [worker]
    info=master:3


For single-machine environment, use the hostname of the machine as both the master and the (only) worker.

For distributed environment, first copy and modify `$Losha_ROOT/scripts/exec.sh` according to actual configuration. `scripts/exec.sh` depends on `pssh`. An example of exec.sh is provided as follows:

    # Parameter
    BIN_DIR=zzzz
    CONF_DIR=zzzz
    MACHINE_CFG="$CONF_DIR/slaves"
    
    echo time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $CONF_DIR && ls -al > /dev/null && cd $BIN_DIR && ls -al > /dev/null && ./$@"
    time pssh -t 0 -P -h ${MACHINE_CFG} -x "-t -t" "cd $CONF_DIR && ls -al > /dev/null && cd $BIN_DIR && ls -al > /dev/null && ./$@"

where slaves is a file that provides the worker machine ID.


## Run a Losha Program

First make sure that the master is running. Use the following to start the master

    $ ./Master --conf /path/to/your/conf

In the single-machine environment, use the following,

    $ ./<executable> --conf /path/to/your/conf

In the distributed environment, use the following to execute workers on all machines,

    $ cp $losha/conf/exec.sh .
    $ ./exec.sh <executable> --conf /path/to/your/conf

## Dataset Preparation
LoSHa accepts FVECS as data formats. Please refer to https://github.com/lijinf2/gqr/tree/master/script for dataset transformations. Specifically, user may follow three steps:

- Transform dataset into FVECS format.
- Use sample_queries.sh to sample a certain amount of queries.
- Use cal_groundtruth.sh to obtain groundtruth.

Specific parameters should be provided to generate the correct datasets.

License
---------------

Copyright 2016-2017 Husky Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
