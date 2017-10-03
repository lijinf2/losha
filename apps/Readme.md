# Applications
In this section, we introduce the usage of various applications avaliable on Losha.

## E2LSH

### Run on a single machine

Build `Master` and `e2lsh`. Assume the root directory of Losha is $Losha.

    $ cd $Losha
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..       # CMAKE_BUILD_TYPE: Release, Debug, RelWithDebInfo
    $ make Master                               # Build Husky master
    $ make e2lsh                                # Build any E2LSH (one application on Losha)

Prepare configuration file.

    $ cd $Losha
    $ vim ./conf/e2lsh.conf

Run the application. You need two terminal.

- On one terminal
    
    $ cd $Losha
    $ cd build 

### Run on multiple machines

## HLSH
