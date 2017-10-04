# Notices

`wb_item.cpp` and `wb_query.cpp` can generate binary query file and item file. 

The data in both .bin and .txt are same. Except that .bin do not contain any empty space or `\n`.

## In .txt

In each line (DV: value in that dimension): 
    
    ID DV DV DV DV DV 

## In .bin

Each chunk contains 6 integer, the first is the ID, then following 5 DV.
