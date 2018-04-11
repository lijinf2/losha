# SimHash
In this section, we introduce the usage of SimHash on Losha.

- How to run simhash with small dataset?
- How to run simhash with glove2.2M dataset?

# Build

Build `Master` and `simhash`. Assume the project root directory is $Losha.

    $ cd $Losha
    $ mkdir build
    $ cd build
    
    # CMAKE_BUILD_TYPE: Release, Debug, RelWithDebInfo
    $ cmake -DCMAKE_BUILD_TYPE=Release ..  
    
    # Build Husky master ('-j4' means using 4 thread to make it, which can be faster)
    $ make -j4 Master
    # Master will be inside ./husky/, move it out                               
    $ mv ./husky/Master ./
    
    # Build SimHash (an application on Losha)
    $ make -j4 simhash

# Run SimHash with a samll dataset

## Step 1. Modify `simhash.cpp` and build SimHash

In the file `$Losha/apps/simhash/simhash.cpp`, comment out some codes for .txt input.

	...
	#include "basic.hpp"
	...
	//#include "glove2m.hpp"
	...
	// INPUT: .txt format
    auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();
    loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, lineInputFormat);
    
    // INPUT: .bin format
    //auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector);
    //loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, binaryInputFormat);
    
Rebuild SimHash

	$ cd $losha/build
	$ make -j4 simhash

## Step 2. Upload small dataset

Upload input files to hdfs and create a folder for output.(remember the path of `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`)

    $ cd $Losha
    $ cd apps/simhash/small_dataset/
    $ hadoop fs -mkdir /[SOME_HDFS_PATH]
    $ hadoop fs -put ./item_small.txt /[SOME_HDFS_PATH]
    $ hadoop fs -put ./query_small.txt /[SOME_HDFS_PATH]
    $ hadoop fs -mkdir /[SOME_HDFS_OUTPUT_PATH]/output

## Step 3. Prepare `simhash.conf`

    $ cd $Losha
    $ cp conf/simhash.conf build/simhash.conf                     # Backup the original .conf file, make your changes in the single.conf
    $ vim build/simhash.conf

Uncomment and modify the following lines (use your own path to replace `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`):

    itemPath=hdfs:///[SOME_HDFS_PATH]/item_small.txt
    queryPath=hdfs:///[SOME_HDFS_PATH]/query_small.txt
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
    
## Step 4. Prepare `machine_cfg.txt`

    $ cd $Losha
    $ cd build/
    $ touch machine_cfg.txt
    $ vim machine_cfg.txt

List all your workers name in it

    worker1
    worker2

## Step 5. Prepare `exec.sh`.

    $ cd $Losha
    $ cp husky/scripts/exec.sh build/
    $ vim build/exec.sh

Modify the the parameters `MACHINE_CFG` and `BIN_DIR`.

    MACHINE_CFG=[SOME_PATH_FROM_ROOT_DIRECTORY]/machine_cfg.txt
    BIN_DIR=[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    
## Step 6. Copy `simhash` And `simhash.conf` To All Machines

Create a same path as `[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/` on all workers. 

Then use scp command to copy `simhash` executable and `simhash.conf` to all machines to that path.

On each machine, the path to the directory, which contains the `simhash` executable and `simhash.conf` file, should be the same.

    # You need to ssh to all your machines and create the directory as we mentioned above. Then you can start the following.
    # Assume you have made e2lsh and multiple.conf on worker1 and you are now in worker1.

    $ cd $Losha
    $ cd build
    
    # FORMAT: $ scp [TARGET_FILE] [YOUR_USER_NAME]@[MACHINE_NAME]: [ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    
    $ scp ./simhash [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.
    $ scp ./simhash.conf [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.
    
## Step 7. Set Up SSH Login Without Password

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

## Step 8. Run the application

You need two terminals. (Maybe you can try `byobu`)

### On terminal A:
    
    $ cd $Losha
    $ cd build 
    $ ./Master --conf ./simhash.conf    

You should see 2 lines print out and one of which says `MASTER READY`.

### On terminal B:

    $ cd $Losha
    $ cd build
    $ ./simhash --conf ./simhash.conf

You can check the output:

    $ hadoop fs -ls /YOUR_HDFS_OUTPUT_PATH/
    $ hadoop fs -cat /YOUR_HDFS_OUTPUT_PATH/part-[XXX]

# Run SimHash with Glove2.2M dataset

## Step 1. Modify `simhash.cpp` and build SimHash

In the file `$Losha/apps/simhash/simhash.cpp`, comment out some codes for .txt input.

	...
	//#include "basic.hpp"
	...
	#include "glove2m.hpp"
	...
	// INPUT: .txt format
    //auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();
    //loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, lineInputFormat);
    
    // INPUT: .bin format
    auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector);
    loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, binaryInputFormat);
    
Rebuild SimHash

	$ cd $losha/build
	$ make -j4 simhash
	
## Step 2. Prepare Glove2.2M dataset

Download the dataset

	$ cd $losha
	$ cd ./apps/simhash/handle_glove2.2m/script/
	$ mkdir raw_data
	$ cd ./raw_data
	$ wget http://nlp.stanford.edu/data/glove.840B.300d.zip
	$ unzip ./glove.840B.300d.zip
	
Convert the data to the format for SimHash. Pick queries from the base file. Using linear search, to find the top-10 nearest neighbors, which we call it `groundtruth` solution.

	# Still under script folder
	$ sh glove_to_fvecs.sh    # It may takes few minutes
	$ sh sample_queries.sh
	$ sh cal_groundtruth.sh

## Step 3. Upload glove2.2m dataset

Upload input files to hdfs and create a folder for output.(remember the path of `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`)

    $ cd $Losha
    $ cd apps/simhash/handle_glove2.2m/script/output
    $ hadoop fs -mkdir /[SOME_HDFS_PATH]
    $ hadoop fs -put ./glove2.2m_base.fvecs /[SOME_HDFS_PATH]
    $ hadoop fs -put ./glove2.2m_query.fvecs /[SOME_HDFS_PATH]
    $ hadoop fs -mkdir /[SOME_HDFS_OUTPUT_PATH]/output

## Step 4. Prepare `simhash.conf`

    $ cd $Losha
    $ cp conf/simhash.conf build/simhash.conf                     # Backup the original .conf file, make your changes in the single.conf
    $ vim build/simhash.conf

Uncomment and modify the following lines (use your own path to replace `[SOME_HDFS_PATH]` and `[SOME_HDFS_OUTPUT_PATH]`):

    itemPath=hdfs:///[SOME_HDFS_PATH]/glove2.2m_base.fvecs
    queryPath=hdfs:///[SOME_HDFS_PATH]/glove2.2m_query.fvecs
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

## Step 5. Prepare `machine_cfg.txt`

    $ cd $Losha
    $ cd build/
    $ touch machine_cfg.txt
    $ vim machine_cfg.txt

List all your workers name in it

    worker1
    worker2

## Step 6. Prepare `exec.sh`.

    $ cd $Losha
    $ cp husky/scripts/exec.sh build/
    $ vim build/exec.sh

Modify the the parameters `MACHINE_CFG` and `BIN_DIR`.

    MACHINE_CFG=[SOME_PATH_FROM_ROOT_DIRECTORY]/machine_cfg.txt
    BIN_DIR=[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/

## Step 7. Copy `simhash` And `simhash.conf` To All Machines

Create a same path as `[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/` on all workers. 

Then use scp command to copy `simhash` executable and `simhash.conf` to all machines to that path.

On each machine, the path to the directory, which contains the `simhash` executable and `simhash.conf` file, should be the same.

    # You need to ssh to all your machines and create the directory as we mentioned above. Then you can start the following.
    # Assume you have made e2lsh and multiple.conf on worker1 and you are now in worker1.

    $ cd $Losha
    $ cd build
    
    # FORMAT: $ scp [TARGET_FILE] [YOUR_USER_NAME]@[MACHINE_NAME]: [ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    
    $ scp ./simhash [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.
    $ scp ./simhash.conf [YOUR_USER_NAME]@worker2:[ANOTHER_PATH_FROM_ROOT_DIRECTORY]/losha/build/
    # It will ask you for password.

## Step 8. Set Up SSH Login Without Password

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


## Step 9. Run The Application 

You need two terminals.

On terminal A:
    
    $ cd $Losha
    $ cd build 
    $ ./Master --conf ./simhash.conf    

You should see 2 lines print out and one of which says `MASTER READY`.

On terminal B:

    $ cd $Losha
    $ cd build
    $ ./exec.sh ./simhash --conf ./simhash.conf

You should see something print out on terminal B.

    SimHash program starts
    ...
    SimHash finish

The information about matched query and item are printed in the output files on hdfs.

You can check it:

    $ hadoop fs -ls /YOUR_HDFS_OUTPUT_PATH/
    $ hadoop fs -cat /YOUR_HDFS_OUTPUT_PATH/part-[XXX]

## Step 10. Compare the result with groundTruth

	$ cd $losha
	$ cd apps/simhash/handle_glove2.2m/script/
	$ mkdir simhash_output
	$ cd simhash_output
	$ hadoop fs -get /YOUR_HDFS_OUTPUT_PATH/part-*
	$ cd ..
	$ vim cal_error_ratio.sh
	
You need to know how many output files in simhash_output/. And then, change the value of variable in `cal_error_ratio`.

	thread_num=NUMBER_OF_OUTPUT_FILES
	
Save and exit Vim.

	$ sh cal_error_ratio.txt
	
The Error Ratio is stored in `./error_ratio.txt`. Check it via:

	$ vim error_ratio.txt

The definition of the error ratio is on the Losha paper.