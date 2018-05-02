#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <utility>

#include "core/engine.hpp"
#include "base/log.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/densevector.hpp"
#include "lshcore/loader/loader.h"

#include "dataobject.h"
#include "adjobject.h"
#include "dataadjhandler.h"
#include "knnagg.h"
// #include "knngraph_train.h"
// #include "knnloader.h"
// #include "block.h"
using namespace husky::losha;
using std::vector;
using std::pair;
using std::string;

void knngraph_train() {
    // initialization
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::string itemPath = husky::Context::get_param("item_path");
    std::string sampleGroundtruthPath = husky::Context::get_param("sample_groundtruth_path");
    int maxIteration = std::stoi(husky::Context::get_param("max_iteration")); 
    int blockPerThread = std::stoi(husky::Context::get_param("block_per_thread"));
    int numHops = std::stoi(husky::Context::get_param("num_hop"));

    typedef float FeatureType;
    // load items
    auto & data_list =
        husky::ObjListStore::create_objlist<DataObject<FeatureType>>();

    DataObject<FeatureType>::loadIdFvecs(data_list, itemPath, dimension);

    // initi adj_list, assume the id ranges from 0 - n - 1, load groundtruth
    auto & adj_list =
        husky::ObjListStore::create_objlist<AdjObject>();

    int maxItemId = count(data_list) - 1;
    DataAdjHandler::buildAdjFromData(data_list, adj_list, sampleGroundtruthPath, maxItemId);

    // // initialize block 
    // auto & block_list =
    //     husky::ObjListStore::create_objlist<Block<FeatureType>>();
    // initBlocks(blockPerThread, block_list);

    // iteration
    for (int i = 0; i < maxIteration; ++i) {
        // build reverse kNN
        // train
        // 1. clustering
        AdjObject::clustering(adj_list, numHops);

        // 2. build block_list and train
        // 
        // train(block_list);

        // 3. get recall
        // cal_sample_avgrecall();
    }
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("dimension");
    args.push_back("item_path"); 
    args.push_back("sample_groundtruth_path"); 
    args.push_back("max_iteration"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(knngraph_train);
        return 0;
    }
    return 1;
}
