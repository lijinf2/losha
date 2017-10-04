# Applications
In this section, we introduce the usage of various applications avaliable on Losha.

## E2LSH

This tutorial talks about the following:

- How to build the application?
- How to run it in single machine environment?
- How to run it in distributed environment?

### Build

Build `Master` and `e2lsh`. Assume the root directory of Losha is $Losha.

    $ cd $Losha
    $ mkdir build
    $ cd build
    
    # CMAKE_BUILD_TYPE: Release, Debug, RelWithDebInfo
    $ cmake -DCMAKE_BUILD_TYPE=Release ..  
    
    # Build Husky master ('-j4' means using 4 thread to make it, which can be faster)
    $ make -j4 Master
    # Master will be inside ./husky/, move it out                               
    $ mv ./husky/Master ./
    
    # Build E2LSH (an application on Losha)
    $ make -j4 e2lsh

### Run in single machine environment

#### Prepare configuration file

    $ cd $Losha
    $ cp conf/e2lsh.conf build/single.conf                     
    # Backup the original .conf file, make your changes in the single.conf

#### Run the application

You need two terminals. (Maybe you can try `byobu`)

##### On terminal A:
    
    $ cd $Losha
    $ cd build 
    $ ./Master --conf ./single.conf    

You should see 2 lines print out and one of which says `MASTER READY`.

##### On terminal B:

    $ cd $Losha
    $ cd build
    $ ./e2lsh --conf ./single.conf

You should see results print out on terminal B. 

`OUTPUT: query_id item_it distance`

Each `OUTPUT` line indicates that the query vector and the item vector are hashed to the same bucket.

    E2LSH program starts
    ...
    OUTPUT: 0 2 0.000000
    OUTPUT: 1 3 0.000000
    OUTPUT: 1 5 1.414214
    OUTPUT: 1 6 1.414214
    ...
    E2LSH finish

### Run in distributed environment

#### Upload Input Files

Upload input files to hdfs and create a folder for output.(remember the path of `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`)

    $ cd $Losha
    $ cd apps/e2lsh/data
    $ hadoop fs -mkdir /[SOME_HDFS_PATH]
    $ hadoop fs -put ./item_ss.txt /[SOME_HDFS_PATH]
    $ hadoop fs -put ./query_ss.txt /[SOME_HDFS_PATH]
    $ hadoop fs -mkdir /[SOME_HDFS_OUTPUT_PATH]/output

#### Prepare `multiple.conf`

    $ cd $Losha
    $ cp conf/e2lsh.conf build/multiple.conf                     # Backup the original .conf file, make your changes in the single.conf
    $ vim build/multiple.conf

Comment out the following lines:
    
    itemPath=nfs://../apps/e2lsh/data/item_ss.txt
    queryPath=nfs://../apps/e2lsh/data/query_ss.txt
    outputPath=localhost

Uncomment and modify the following lines (use your own path to replace `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`):

    itemPath=hdfs:///[SOME_HDFS_PATH]/item_ss.txt
    queryPath=hdfs:///[SOME_HDFS_PATH]/query_ss.txt
    outputPath=/[SOME_HDFS_OUTPUT_PATH]/output

Modify the cluster configuration. Assume that you have two workers, worker1 and worker2. And you will start running the `Master` and `E2LSH` executable from worker1.
    
    master_host=worker1
    hdfs_namenode=worker1
    hostname=worker1
    [worker]
    info=worker1:2              
    # The above2 means run 2 threads, you can change it to other number
    info=worker2:2             
    
    # You can check how many cpu you have on that machine and then, decide how many threads you want to use. If you have more machines, you can add more. (e.g. info=worker3:5)

#### Prepare `machine_cfg.txt`

    $ cd $Losha
    $ cd build/
    $ touch machine_cfg.txt
    $ vim machine_cfg.txt

List all your workers name in it

    worker1
    worker2

#### Prepare `exec.sh`.

    $ cd $Losha
    $ cp husky/scripts/exec.sh build/
    $ vim build/exec.sh

Modify the the parameters `MACHINE_CFG` and `BIN_DIR`.

    MACHINE_CFG=[SOME_PATH_FROM_ROOT_DIRECTORY]/machine_cfg.txt
    BIN_DIR=[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/

#### Copy `e2lsh` And `multiple.conf` To All Machines

Create a same path as `[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/` on all workers. 

Then use scp command to copy `e2lsh` executable and `multiple.conf` to all machines to that path.

On each machine, the path to the directory, which contains the `e2lsh` executable and `multiple.conf` file, should be the same.

    # You need to ssh to all your machines and create the directory as we mentioned above. Then you can start the following.
    # Assume you have made e2lsh and multiple.conf on worker1 and you are now in worker1.

    $ cd $Losha
    $ cd build
    
    # FORMAT: $ scp [TARGET_FILE] [YOUR_USER_NAME]@[MACHINE_NAME]: [ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    
    $ scp ./e2lsh [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.
    $ scp ./multiple.conf [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.

#### Set Up SSH Login Without Password

You should be able to access to all machines without using password.

To achieve this purpose, you will need to generate authentication keys. Please refer to [this tutorial](http://www.linuxproblem.org/art_9.html).
	
	# If, after finishing this tutorial, you still fail to ssh other machines without password, try the followings:
	1. chmod 700 ~/.ssh
	2. chmod 644 ~/.ssh/authorized_keys
	
** ATTENTION! Assume you are in worker1, you should also be able to `ssh worker1`! **

If you successfully set this up, you should be able to ssh to other machines without using password.

	# Assume you are in worker1 and you have two workers, worker1 and worker2. Then you should be able to ssh to both of them
	$ ssh worker2
	# Successfully login to worker2, without using password.
	$ ssh worker1
	# Successfully login to worker1, without using password.


#### Run The Application 

You need two terminals.

On terminal A:
    
    $ cd $Losha
    $ cd build 
    $ ./Master --conf ./multiple.conf    

You should see 2 lines print out and one of which says `MASTER READY`.

On terminal B:

    $ cd $Losha
    $ cd build
    $ ./exec.sh ./e2lsh --conf ./multiple.conf

You should see something print out on terminal B.

    E2LSH program starts
    ...
    E2LSH finish

The information about matched query and item are printed in the output files on hdfs.

You can check it:

    $ hadoop fs -ls /YOUR_HDFS_OUTPUT_PATH/
    $ hadoop fs -cat /YOUR_HDFS_OUTPUT_PATH/part-[XXX]

### What is more?

Want to try binary file as input dataset? Please refer to `losha/apps/e2lsh/README.md`.

Want to know more about the details of input dataset? Please refer to `losha/apps/e2lsh/data/README.md`.

## HLSH

TO DO...
