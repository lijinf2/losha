//
// Created by darxan on 2018/4/23.
//
#include <vector>
#include <queue>
#include "io/input/inputformat_store.hpp"


#include "lshcore/loader/loader.h"
#include "losha/query/default.hpp"
#include "linearscan.h"

void linearScan() {
        hushk::losha::LinearScanner scanner;
        scanner.linearScan();
};


int main(int argc, const char ** argc) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("dimension");
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath

    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(linearScan);
        return 0;
    }
    return 1;
}