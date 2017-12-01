# Image retrieval demo using Losha

## Concept

With Losha, this demo presents an image retrieval application to utilize distributed fast neighbor search. It can look for the original piece of image within a directory full of images given part of the image that could have been altered and cropped.

![](https://d2mxuefqeaa7sj.cloudfront.net/s_4059602D3E424BE76E9ADEF542E217E1EE55739753AB83D553C21A1C1D57147B_1512058775305_demo.png)

## Build

**Prerequisites**

- OpenCV (with opencv_contri) 3.3.1

Build the two apps, **Img2Sift** and **create_match** with the following:

```bash
g++ -std=c++11 Img2Sift.cpp -o Img2Sift `pkg-config opencv --cflags --libs`
g++ -std=c++11 create_match.cpp -o create_match `pkg-config opencv --cflags --libs`
```

It is also required to make E2LSH_IR app, a modified version of E2LSH with:

```bash
cd build
make -j{N} E2LSH_IR
```    


## Usage

### Overview
There are three steps to complete the process:

1. Create Image Binary
2. Run nearest neighbor search (NNS)
3. Match image

#### Step 1: Create Image Binary
Img2Sift provides functions to extract SIFT from image. 
Currently, the number of images in the directory is limited to 1000 images. 


- To show usage:
``` bash
./Img2Sift --help
```

- To generate single file binary (usually the query image):
```bash
./Img2Sift <image_file_name> <output_file_name>
```

- To generate binary for all images within a directory (usually the items)
```bash
./Img2Sift <number_of_files> <directory_name> <output_file_name>
```

#### Step 2: Run NNS
In order to run NNS, we need to update the configuration file for the Data management Paths, due to the large amount of query, please place it on hdfs:

```txt
# Data management
outputPath=<Path_to_output_results>
itemPath=hdfs:///<Path_to_items>
queryPath=hdfs:///<Path_to_query>
```

A good set of parameter for IR can be configured as follow:

```txt
band=10
row=20
W=600
dimension=128
maxIteration=1
```

With two terminals run:

```bash
./Master --conf /path/to/your/conf
```
```bash
./exec.sh e2lsh_ir --conf /path/to/your/conf
```

#### Step 3: Match image
In a distributed environment，pull the output results and concat into a single discription file.
```bash
hadoop fs -get hdfs://<output_path> .
cat <output_path>/* >> <description_file_name>
```

with the description, we can execute create_match. create_match provides two functions: matching the query image with another single image, or with the entire directory. Both creates a matching-image as result.


- To pair two images, run:
```bash
./create_match -two <query_image> <item_image> <description_file_name>
```

- To find the image within the directory:
```bash
./create_match -dir <query_image> <directory_name> <description_file_name>
```

## Example

In this demo folder, a sample is provided with all intermediate files. The folder structure is presented as following:

```txt
    demo/
    ├── results/
    ├── data/
    │   ├── dir/
    │   ├── inter/
```

Within the data folder, we query the `duck.jpg` toward the images within the `dir` folder which contains the original duck image and nine other random images.

**Step 1 :**
create the binary for query image duck.jpg
```bash
./Img2Sift.cpp duck.jpg nov.duck
```

the intermediate files is provided within `data/dir/inter/`
Then, create the binary for the entire directory
```bash
./Img2Sift.cpp 10 dir nov.dir
```
the intermediate files is provided within `data/dir/inter/`


**Step 2 :**
place onto hdfs and run E2LSH_IR 

```bash
#in one terminal
./Master --conf /path/to/conf
```
```bash
#in another terminal
hdfs -put nov.dir nov.duck /<input_location>
./exec.sh e2lsh_ir --conf /path/to/conf
hadoop fs -get hdfs://output .
cat output/* >> all.txt
```
the intermediate file `all.txt` is provided within `data/dir/inter/`

**Step 3 :**
run create_match
```bash
./create_match -dir duck.jpg dir all.txt
```

## Result

With the contrast and lightening altered duck.jpg, we can correctly find the original image within the directory.

![](https://d2mxuefqeaa7sj.cloudfront.net/s_4059602D3E424BE76E9ADEF542E217E1EE55739753AB83D553C21A1C1D57147B_1512100429015_dir.jpg)

## More information & Acknowledgements

This application is part of the Final Year Project by tcheng and the detailed documentation is available [here](https://tichung.com/static/Yr4_CSCI4999_FYP_report.pdf).
Some other results and performance details can be found here (under construction).
The image here are obtained from the internet under [Creative Common CC0 licence](https://creativecommons.org/publicdomain/zero/1.0/deed) and will remain as it is.
