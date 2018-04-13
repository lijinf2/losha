# Losha is a framework focusing on Distributed Similarity Search

## Apps
  - simhash
  - e2lsh
  - gqr + pca hashing

## Dependency
  - GQR (https://github.com/lijinf2/gqr)
  - Husky (https://github.com/husky-team/husky)

## Build and Run

We assume you have set up a NFS director (denoeted as /data) that every machine can access it. 

### Make runnable binary

    $ cd /data
    $ git clone --recursive https://github.com/lijinf2/losha.git
    $ cd losha
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..  
    $ make help                     
    $ make -j4 Master
    $ make -j4 e2lsh 

### Prepare dataset and groundtruth
    $ cd ../script
    $ python idfvecs.py
    $ hadoop dfs -mkdir /losha /losha/audio
    $ hadoop dfs -Ddfs.blocksize=8388608 -put audio_base.idfvecs /losha/audio
    $ hadoop dfs -Ddfs.blocksize=8388608 -put audio_query.idfvecs /losha/audio
    $ cd ../gqr/ && mkdir build && cd ./script
    $ sh cal_groundtruth.sh

### Run in single machine (assume the hostname is master)
    $ cd ../../script
    $ ../build/husky/Master --conf ../conf/e2lsh.conf
    $ ../build/e2lsh --conf ../conf/e2lsh.conf
    $ sh evaluate.sh

### Run in multiple machines (assume 2 worker machines, with hostnames as worker1 and worker2)
    $ ../build/husky/Master --conf ../conf/e2lsh-slaves.conf
    $ ./exec.sh ../build/e2lsh --conf ../conf/e2lsh-slaves.conf
    $ sh evaluate.sh

## Tips
    $ if you change the root directory (i.e. /data/losha) to other position, please change shell scripts correspondingly
    $ remove outputfile on HDFS, otherwise the old results will co-exist with the new results

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
