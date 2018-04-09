#include <cmath>
#include <limits>
#include <vector>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshengine.hpp"
#include "lshcore/e2lshfactory.hpp"
#include "lshcore/loader/loader.h"

#include "e2lsh.hpp"
typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket;
using namespace husky::losha;
E2LSHFactory<ItemIdType, ItemElementType> factory;
std::once_flag factory_flag;

void lsh() {

    // initialization
    int band = std::stoi(husky::Context::get_param("band"));
    int row = std::stoi(husky::Context::get_param("row"));
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    int W = std::stoi(husky::Context::get_param("W"));
    std::call_once(factory_flag, [&]() {
        factory.initialize(band, row, dimension, W);
    });

    int BytesPerVector = dimension * 4 + 8;
    auto& binaryInputFormat = 
       husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    loshaengine<Query, Bucket, Item, QueryMsg, AnswerMsg>(
       factory, parseIdFvecs, binaryInputFormat);
}

int main(int argc, char ** argv) {
    husky::LOG_I << "E2LSH program starts" << std::endl;
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("band");
    args.push_back("row");
    args.push_back("dimension");
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath
    args.push_back("W");
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}

