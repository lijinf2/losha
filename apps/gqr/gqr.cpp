#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshbucket.hpp"
#include "lshcore/lshengine.hpp"
#include "lshcore/loader/loader.h"
#include "lshcore/lshfactory/pcafactory.hpp"
#include "gqr.hpp"
using namespace husky::losha;

typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
typedef GQRQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query;
typedef GQRItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item;
typedef LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket;
PCAFactory<ItemIdType, ItemElementType> factory;
std::once_flag factory_flag;

void lsh() {

    // initialization
    std::string modelFilePath = husky::Context::get_param("modelFile");
    std::call_once(factory_flag, [&modelFilePath]() {
        factory.initialize(modelFilePath);
    });

    int BytesPerVector = factory.getDimension() * 4 + 8;
    std::string itemPath = husky::Context::get_param("itemPath");
    std::string queryPath = husky::Context::get_param("queryPath");
    int numIteration = std::stoi(husky::Context::get_param("maxIteration"));
    auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    loshaengine<Query, Bucket, Item, QueryMsg, AnswerMsg>(factory, parseIdFvecs, binaryInputFormat, itemPath, queryPath, numIteration);
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("modelFile"); // the path of pcah modelfile
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath
    args.push_back("maxIteration");
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}
