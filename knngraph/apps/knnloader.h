#pragma once
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include "core/engine.hpp"
#include "boost/tokenizer.hpp"
#include "knnvertex.h"
#include "block.h"
using std::string;
using std::vector;
using std::pair;
using std::istringstream;

namespace husky {
namespace losha {
template<typename FeatureType>
void loadIdFvecs(
    husky::ObjList<KNNVertex<FeatureType>>& obj_list,
    const string& itemPath, 
    int dimension) {

    int BytesPerVector = dimension * sizeof(FeatureType) + 8;
    auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    binaryInputFormat.set_input(itemPath);

    auto& loadChannel = 
        husky::ChannelStore::create_push_channel<
           vector<FeatureType>>(binaryInputFormat, obj_list);
    husky::load(binaryInputFormat, item_loader(loadChannel, parseIdFvecs<FeatureType>));

    husky::list_execute(obj_list,
        [&loadChannel, &dimension](KNNVertex<FeatureType>& obj){
        auto msgs = loadChannel.get(obj);
        assert(msgs.size() == 1);
        obj.setItemVector(msgs[0]);
        assert(obj.getItemVector().size() == dimension);
    });

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finish loading from itemPath: " << itemPath << std::endl;
    }
}

template<typename FeatureType>
void loadSampleGroundtruth(
    const string& inputPath, 
    husky::ObjList<KNNVertex<FeatureType>>& obj_list) {

    // load file and send message to KNNVertex list
    auto& lineInputformat =
        husky::io::InputFormatStore::create_line_inputformat();
    lineInputformat.set_input(inputPath);

    auto& channel = 
        husky::ChannelStore::create_push_channel<
           vector<pair<int, float>>>(lineInputformat, obj_list);

    auto parser = [&channel](boost::string_ref& line) {
        boost::char_separator<char> sep(" \t");
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        int size = 0;
        for (auto& w : tok) {
            size++;
        }
        if (size <= 2) return;

        int queryId;
        int itemId;
        float distance;
        vector<pair<int, float>> knn;

        auto it = tok.begin();
        queryId = std::stoi(*it++);
        // abandon the first pair, in which itemId is queryId, and the distance is 0;
        it++;
        it++;
        while(it != tok.end()) {
            itemId = std::stoi(*it++);
            distance = std::stof(*it++);
            knn.emplace_back(std::make_pair(itemId, distance));
        }
        channel.push(knn, queryId);
    };
    husky::load(lineInputformat, parser);

    // build trueKNN in KNNVeretx
    husky::list_execute(obj_list,
        [&channel](KNNVertex<FeatureType>& obj){
        const auto msgs = channel.get(obj);
        if (msgs.size() == 0)
            return;
        else
            assert(msgs.size() == 1);
        obj.setTrueKNN(msgs[0]);
    });
}

template<typename FeatureType>
void initBlocks(
    int blockPerThread, 
    husky::ObjList<Block<FeatureType>>& block_list) {
}
}
