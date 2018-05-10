#include <cmath>
#include <limits>
#include <vector>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshengine.hpp"
#include "lshcore/loader/loader.h"

#include "losha/query/default.hpp"
#include "lshcore/e2lshfactory.hpp"
#include "linearscan.h"
using namespace husky::losha;

typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
typedef LSQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query;
typedef LSItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item;
typedef DefaultBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket;
E2LSHFactory<ItemIdType, ItemElementType> factory;
std::once_flag factory_flag;

void lsh() {

    // initialization
    std::call_once(factory_flag, [&]() {
        factory.initialize(0, 0, 0, 1.0);
    });
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    int BytesPerVector = dimension * 4 + 8;
    std::string itemPath = husky::Context::get_param("itemPath");
    std::string queryPath = husky::Context::get_param("queryPath");
    int numIteration = 2;
    auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    loshaengine<Query, Bucket, Item, QueryMsg, AnswerMsg>(factory, parseIdFvecs, binaryInputFormat, itemPath, queryPath, numIteration);
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("dimension");
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath
    args.push_back("topK"); // the outputPath
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}
