#include <cmath>
#include <vector>
#include <utility>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshengine.hpp"
#include "lshcore/loader/loader.h"

#include "losha/query/default.hpp"
#include "lshcore/lshfactory/apsimhashfactory.hpp"
using namespace husky::losha;

typedef int ItemIdType;
typedef std::pair<int, float> ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, float> AnswerMsg;
typedef DefaultQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query;
typedef DefaultItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item;
typedef DefaultBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket;

APSparseSimHashFactory<int, float> factory;
std::once_flag factory_flag;

void lsh() {

    // initialization
    int band = std::stoi(husky::Context::get_param("band"));
    int row = std::stoi(husky::Context::get_param("row"));
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::call_once(factory_flag, [&]() {
        factory.initialize(band, row, dimension);
    });

    std::string itemPath = husky::Context::get_param("itemPath");
    std::string queryPath = husky::Context::get_param("queryPath");
    auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();
    loshaengine<Query, Bucket, Item, QueryMsg, AnswerMsg>(factory, parseIdLibsvm, lineInputFormat, itemPath, queryPath);
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("band");
    args.push_back("row");
    args.push_back("dimension");
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}

