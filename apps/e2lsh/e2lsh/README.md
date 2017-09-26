# Play with E2LSH using small datasets

We have two small datasets, which are aimed to help you get familiar with the framework. 

The two datasets, one is in .txt format, another is in binary format. The item and query in both format are the same.

You can set the itemPath and queryPath in `losha/conf/e2lsh.conf`. By default, they are point to `losha/apps/e2lsh/data/item_ss.txt` and `losha/apps/e2lsh/data/query_ss.txt`.


## TXT format (default input setting in e2lsh.cpp) 

Query file: `losha/apps/e2lsh/data/item_ss.txt`
    
Item file: `losha/apps/e2lsh/data/query_ss.txt`


### Using this dataset:
        
Change `losha/conf/e2lsh.conf`, set the `itemPath` and `queryPath` correctly. Set `outputPath` correctly (which should be on HDFS).
        
Change in `./e2lsh.cpp`:
            
- Using `#include "small.hpp"`
            
- Comment out `#include "small_binary.hpp"`
            
- Using `auto& lineInputFormat ...`
            
- Comment out `auto& binaryInputFormat ...`
            
- Using `loshaengine ... setItemSMALL, lineInputFormat);`
            
- Comment out `loshaengine ... setItemSmallBinary, binaryInputFormat);`
        
In `losha/build/` directory, run `make e2lsh`.


## Binary format

Query file: `losha/apps/e2lsh/data/item_ss.bin`

Item file: `losha/apps/e2lsh/data/query_ss.bin`


### Using this dataset:
    
Change `losha/conf/e2lsh.conf`, set the `itemPath` and `queryPath` correctly. Set `outputPath` correctly (which should be on HDFS).

Change in `./e2lsh.cpp`:
        
- Comment out `#include "small.hpp"`
        
- Using `#include "small_binary.hpp"`
        
- Comment out `auto& lineInputFormat ...`
        
- Using `auto& binaryInputFormat ...`
        
- Comment out `loshaengine ... setItemSMALL, lineInputFormat);`
        
- Using `loshaengine ... setItemSmallBinary, binaryInputFormat);`
    
In `losha/build/` directory, run `make e2lsh`.
