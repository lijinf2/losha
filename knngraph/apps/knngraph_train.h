#pragma once
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
#include "losha/common/aggre.hpp"
#include "gqr/util/random.h"

#include "dataobject.h"
#include "adjobject.h"
#include "dataadjhandler.h"
#include "block.h"
#include "dataagg.h"
// #include "knngraph_train.h"
// #include "knnloader.h"
using namespace husky::losha;
using std::vector;
using std::pair;
using std::string;
using std::unordered_set;

namespace husky {
namespace losha {

template<typename FeatureType>
void knngraph_train(
        int dimension,
        string itemPath, 
        string sampleGroundtruthPath,
        int maxIteration,
        int numBlocks,
        int numNBPerNode,
        std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor
        ) {
    // load items
    auto & data_list =
        husky::ObjListStore::create_objlist<DataObject<FeatureType>>();

    DataObject<FeatureType>::loadIdFvecs(data_list, itemPath, dimension);
    int numData = count(data_list);

    // initi adj_list, assume the id ranges from 0 - n - 1, load groundtruth
    auto & adj_list =
        husky::ObjListStore::create_objlist<AdjObject>();

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

    DataAgg<FeatureType> dataset(data_list, numData);
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
        unordered_set<unsigned> labels = sampleRand(numData, numBlocks);
        AdjObject::randomClustering(adj_list, labels);
        
        // unordered_set<unsigned> labels = sampleRand(numData, numBlocks);
        // unordered_set<unsigned> labels = AdjObject::getKIdMaxIndegree(adj_list, numBlocks);
        // AdjObject::bfsClustering(adj_list, labels);

        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "iteration " << i << " starts knn graph training" << std::endl;
        }

        // 2. build block_list and train
        // #cc = #blocks
        // Block::train(adj_list, data_list, distor);
        //
        Block::trainFitMem(adj_list, dataset, distor);

        // 3. get recall
        float avgRecall = AdjObject::calSampleAvgRecall(adj_list);
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "iteration " << i <<  " finished. The avg recall is " << avgRecall << std::endl;
            husky::LOG_I << std::endl;
        }
    }
}
}
}
