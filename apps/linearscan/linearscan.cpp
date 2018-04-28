//
// Created by darxan on 2018/4/23.
//
#include <vector>
#include <queue>

#include "losha/query/default.hpp"
#include "linearscan.h"

typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
typedef DefaultQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query;
typedef DefaultItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item;

void linearScan() {

        husky::losha::LinearScanner<Query, Item, ItemIdType, ItemElementType> scanner;
        scanner.linearScan();
};


int main(int argc, char ** argv) {
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
