#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <utility>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/densevector.hpp"
#include "lshcore/loader/loader.h"

#include "knnvertex.h"
#include "knnloader.h"
#include "knnagg.h"
#include "knngraph_train.h"
#include "block.h"
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

    typedef float FeatureType;
    // load items
    auto & item_list =
        husky::ObjListStore::create_objlist<KNNVertex<FeatureType>>();

    loadIdFvecs(item_list, itemPath, dimension);

    // load groundtruth
    loadSampleGroundtruth(sampleGroundtruthPath, item_list);

    // initialize knngraph, assume the id ranges from 0 to n - 1
    int maxItemId = count(item_list) - 1;
    husky::list_execute(item_list,
        [&maxItemId](KNNVertex<FeatureType>& item){
        item.initFoundKNN(maxItemId);
    });

    // initialize block 
    auto & block_list =
        husky::ObjListStore::create_objlist<Block<FeatureType>>();
    initBlocks(blockPerThread, block_list);

    // iteration
    for (int i = 0; i < maxIteration; ++i) {
        // train
        // 1. clustering
        clustering(item_list, block_list);

        // 2. getItem and train

        // 3. all-pair comparison

        // get recall
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
