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

### Run in single machine (assume the hostname is master, Master run in one terminal and e2lsh runs in another terminal)
    $ cd ../../script
    $ ../build/husky/Master --conf ../conf/e2lsh.conf
    $ // open another shell to run e2lsh
    $ ../build/e2lsh --conf ../conf/e2lsh.conf
    $ sh evaluate.sh

### Run in multiple machines (assume 2 worker machines, with hostnames as worker1 and worker2)
    $ ../build/husky/Master --conf ../conf/e2lsh-slaves.conf
    $ ./exec.sh ../build/e2lsh --conf ../conf/e2lsh-slaves.conf
    $ sh evaluate.sh

## Tips
    - Master and e2lsh run on two different shells
    - Always remove output files on HDFS before next try.
    - We suggest to set up GQR and Husky before using LoSHa. 

## Reference

**[LoSHa: A General Framework for Scalable Locality Sensitive Hashing (SIGIR 2017)](http://appsrv.cse.cuhk.edu.hk/~jfli/paper/2017/losha.pdf)**
