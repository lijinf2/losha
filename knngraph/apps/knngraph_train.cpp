#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>

#include "core/engine.hpp"
#include "base/log.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/densevector.hpp"
#include "lshcore/loader/loader.h"
#include "losha/common/writer.hpp"
#include "gqr/util/random.h"

#include "dataobject.h"
#include "adjobject.h"
#include "dataadjhandler.h"
#include "knnagg.h"
#include "block.h"
// #include "knngraph_train.h"
// #include "knnloader.h"
using namespace husky::losha;
using std::vector;
using std::pair;
using std::string;
using std::unordered_set;

void knngraph_train() {
    // initialization
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::string itemPath = husky::Context::get_param("item_path");
    std::string sampleGroundtruthPath = husky::Context::get_param("sample_groundtruth_path");
    int maxIteration = std::stoi(husky::Context::get_param("max_iteration")); 
    int numBlocks = std::stoi(husky::Context::get_param("num_block"));
    // int numHops = std::stoi(husky::Context::get_param("num_hop"));
    int numNBPerNode = std::stoi(husky::Context::get_param("K"));

    typedef float FeatureType;
    // load items
    auto & data_list =
        husky::ObjListStore::create_objlist<DataObject<FeatureType>>();

    DataObject<FeatureType>::loadIdFvecs(data_list, itemPath, dimension);

    // initi adj_list, assume the id ranges from 0 - n - 1, load groundtruth
    auto & adj_list =
        husky::ObjListStore::create_objlist<AdjObject>();

    int numData = count(data_list);
    int maxItemId = numData - 1;
    DataAdjHandler::buildAdjFromData(data_list, adj_list, sampleGroundtruthPath, maxItemId, numNBPerNode);

    // debug infor
    // husky::list_execute(
    //     adj_list, 
    //     [](AdjObject& adj) {
    //     string str = std::to_string(adj.id());
    //     str += " ";
    //     str += std::to_string(adj._foundKNN.size());
    //     for (int i = 0; i < adj._foundKNN.size(); ++i) {
    //         str += " ";  
    //         str += std::to_string(adj._foundKNN[i].first);
    //         // str += std::to_string(adj._rKNN[i]);
    //     }
    //     str += "\n";
    //     writeHDFS(str, "hdfs_namenode", "hdfs_namenode_port", "tmp_output_path");
    // });
    // should debug
    // auto tmp = AdjObject::getRKNNSizeMaxMin(adj_list);

    // iteration
    unordered_set<unsigned> labels = sampleRand(numData, numBlocks);
    for (int i = 0; i < maxIteration; ++i) {
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "start iteration " << i << std::endl;
        }

        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "iteration " << i << " starts clustering " << std::endl;
        }
        // build reverse kNN
        // train
        // 1. clustering
        // AdjObject::randomClustering(adj_list, labels);
        
        AdjObject::bfsClustering(adj_list, labels);

        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "iteration " << i << " starts knn graph training" << std::endl;
        }
        // 2. build block_list and train
        // #cc = #blocks
        Block::train(adj_list, data_list);

        // 3. get recall
        float avgRecall = AdjObject::calSampleAvgRecall(adj_list);
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "iteration " << i <<  " finished. The avg recall is " << avgRecall << std::endl;
            husky::LOG_I << std::endl;
        }
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
    args.push_back("tmp_output_path"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(knngraph_train);
        return 0;
    }
    return 1;
}
