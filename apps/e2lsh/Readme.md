# Distributed E2LSH

Exact Euclidean Locality Sensitive Hashing (E2LSH) was proposed by Alexandr Andoni and Piotr Indyk. Here we perform a distributed construction of E2LSH. This method utilizes euclidean distance metrics to perform approximate nearest neighbor search. Here we present an example for using Distributed E2LSH (DE2LSH) running on Losha.

## Build

```bash
make -j[N] e2lsh
```

## Example

The following demonstrates the usage of e2lsh and configuration using a small data set from [TEXMEX](http://corpus-texmex.irisa.fr/). To be precise, ANN_SIFT10K is a small binary data set consisting 128 dimensions of float data types. We would perform the query file on itself.

## Data preprocessing

The [siftsmall_query.fvecs raw data](ftp://ftp.irisa.fr/local/texmex/corpus/siftsmall.tar.gz) does not contain an ID for Losha to process. Thus we need to run the data through our util `ANNtransform` with the following command.

```bash
./ANNtransform sift10 siftsmall_query.fvecs smallq.fvecs
```

`smallq.fvecs` is the transformed file for processing.

## Code Configuration

`e2lsh.cpp` is the main function that controls DE2LSH. 

`gist1m.hpp`, `sift10k.hpp` and `sift1b.hpp` are files that define the different data set format.

```c++
// in e2lsh.cpp //

loshaengine <Query10K, Bucket10K, Item10K, QueryMsg, AnswerMsg> (factory, setItemSIFT10K, binaryInputFormat);
```

```c++
// in sift10k.hpp
typedef float ItemElementType;
const int idBytes_sift10K = sizeof(ItemIdType); //int
const int dimension_sift10K = 128;

//...//

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query10K;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item10K;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket10K;

const int BytesPerVector = idBytes_sift10K + dimension_sift10K * elementBytes_sift10K;
```

We can see that the two files define the data set format coherently.

## Config File

```text
outputPath= <pathToOutput>

itemPath=hdfs:///<PATH>/smallq.fvecs
queryPath=hdfs:///<PATH>/smallq.fvecs

# E2LSH setting
band=10
row=20
W=600
dimension=128
maxIteration=1

# the following is for cluster configuration
master_host=master
master_port=<portNumber>
comm_port=<portNumber>

hdfs_namenode=master
hdfs_namenode_port=<port>

serve=1
hostname=localhost
port=<port>

[worker]
info=w20:1
```

Here we set our DE2LSH with band, row, width (W) and dimension accordingly. These parameter can be fine-tuned according to different data sets.
Here, the config file is also setup on a distributed setting (which is worker20).
Don't forget to set the works accordingly to `slave`.

## Running the Application

```bash
./Master --conf path/to/conf/e2lsh.conf
```

```bash
../conf/exec.sh ./e2lsh --conf path/to/conf/e2lsh.conf
```

## Results

A snippet of the results is as following:

```text
//some more results ... //
72 72 0.000000
73 73 0.000000
74 74 0.000000
75 75 0.000000
76 76 0.000000
77 77 0.000000
78 78 0.000000
79 79 0.000000
80 80 0.000000
81 81 0.000000
82 82 0.000000
83 83 0.000000
//some more results ... //
```

Since we are running nearest neighbor against itself, the nearest is the same entry.


#License

Copyright 2016-2017 Husky Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
