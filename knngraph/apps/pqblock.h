#pragma once
#include <utility>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <limits>

#include "core/engine.hpp"

#include "losha/common/aggre.hpp"

#include "dataobject.h"
#include "adjobject.h"
using std::pair;
using std::vector;
using std::unordered_map;

namespace husky {
namespace losha {

class PQBlock {
public:
    using KeyT = pair<int, int>;
    KeyT _center;
    explicit PQBlock(const KeyT& id) : _center(id) {};
    const KeyT id() const {return _center;}

    vector<int> _itemIds;
};

template<typename FeatureType>
class PQBlockAgg {
public:
    // typedef pair<int, vector<FeatureType>> RecordT;
    typedef std::pair<int, std::vector<FeatureType>> IdVectorPair;
    PQBlockAgg(
        husky::ObjList<DataObject<FeatureType>>& data_list,
        std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor,
        int numData);

    static void initAdjList(
        husky::ObjList<DataObject<FeatureType>>& data_list,
        husky::ObjList<AdjObject>& adj_list,
        PQBlockAgg& pqba, 
        std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor,
        int numVisitedBlocksExpected = 4);

    vector<IdVectorPair>& getLeftCenters() {
        return _leftAgg.get_value();
    }

    vector<IdVectorPair>& getRightCenters() {
        return _rightAgg.get_value();
    }
protected:
    husky::lib::Aggregator<vector<IdVectorPair>> _leftAgg;
    husky::lib::Aggregator<vector<IdVectorPair>> _rightAgg;
    unordered_set<unsigned> _leftLabels;
    unordered_set<unsigned> _rightLabels;

    husky::ObjList<PQBlock>* _pq_list = nullptr;
    husky::PushChannel<int, PQBlock>* _data2pqblock = nullptr;
};

template<typename FeatureType>
void PQBlockAgg<FeatureType>::initAdjList(
    husky::ObjList<DataObject<FeatureType>>& data_list,
    husky::ObjList<AdjObject>& adj_list,
    PQBlockAgg& handler,
    std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor,
    int numVisitedBlocksExpected) {

    // IMI request
    auto& pq_list = *(handler._pq_list);
    auto& data2pqblock = *(handler._data2pqblock);
    auto& leftCenters = handler.getLeftCenters();
    auto& rightCenters = handler.getRightCenters();
    int numBlocksPerBook = (int)sqrt(numVisitedBlocksExpected);
    vector<pair<float, int>> leftBlocks, rightBlocks;
    leftBlocks.reserve(numBlocksPerBook);
    rightBlocks.reserve(numBlocksPerBook);

    husky::list_execute(
        data_list, 
        {},
        {&data2pqblock},
        [&data2pqblock, &leftCenters, &rightCenters, 
         &leftBlocks, &rightBlocks, &distor, numBlocksPerBook](DataObject<FeatureType>& data){

        for (int i = 0; i < leftCenters.size(); ++i) {
            const IdVectorPair& center = leftCenters[i];
            float dist = distor(data.getItemVector(), center.second);
            if (leftBlocks.size() < numBlocksPerBook 
                || dist < leftBlocks.front().first) {
                leftBlocks.emplace_back(std::make_pair(dist, center.first));
                std::push_heap(leftBlocks.begin(), leftBlocks.end());
            }

            if (leftBlocks.size() > numBlocksPerBook) {
                std::pop_heap(leftBlocks.begin(), leftBlocks.end());
                leftBlocks.pop_back();
            }
        }

        for (int i = 0; i < rightCenters.size(); ++i) {
            const IdVectorPair& center = rightCenters[i];
            float dist = distor(data.getItemVector(), center.second);
            if (rightBlocks.size() < numBlocksPerBook 
                || dist < rightBlocks.front().first) {
                rightBlocks.emplace_back(std::make_pair(dist, center.first));
                std::push_heap(rightBlocks.begin(), rightBlocks.end());
            } 
            if (rightBlocks.size() > numBlocksPerBook) {
                std::pop_heap(rightBlocks.begin(), rightBlocks.end());
                rightBlocks.pop_back();
            }
        }

        pair<int, int> pqblock;
        for (auto lid : leftBlocks) {
            pqblock.first = lid.second;
            for (auto rid : rightBlocks) {
                pqblock.second = rid.second;
                data2pqblock.push(data.id(), pqblock);
            }
        }
        leftBlocks.clear();
        rightBlocks.clear();
    });

    // pq_list response to adj_list
    thread_local auto& pqblock2adj = 
        husky::ChannelStore::create_push_channel<int>(pq_list, adj_list);

    husky::list_execute(
        pq_list,
        {&data2pqblock},
        {&pqblock2adj},
        [&data2pqblock, &pqblock2adj](PQBlock& block){
        for (auto dataId : data2pqblock.get(block)) {
            for (auto itemId : block._itemIds) {
                pqblock2adj.push(itemId, dataId);
            }
        }
     });

    // update to adj_list
    husky::list_execute(
        adj_list, 
        {&pqblock2adj},
        {},
        [&pqblock2adj](AdjObject& adj){
            const auto& msgs = pqblock2adj.get(adj);
            int index = 0;
            for (auto id : msgs) {
                if (index >= adj._foundKNN.size()) break;
                if (id != adj.id()) {
                    adj._foundKNN[index++].first = id;
                }
            }
            int size = adj._foundKNN.size();
        }
    );

    AdjObject::updateRKNN(adj_list);
};

template<typename FeatureType>
PQBlockAgg<FeatureType>::PQBlockAgg(
    husky::ObjList<DataObject<FeatureType>>& data_list,
    std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor,
    int numData) : 
        _leftAgg (vectorAggs<IdVectorPair>(1)[0]),
        _rightAgg (vectorAggs<IdVectorPair>(1)[0]) {

    int codebookSize = sqrt(numData) * 2;
    _leftLabels = sampleRand(numData, codebookSize, numData);
    _rightLabels = sampleRand(numData, codebookSize, numData * 2);

    // broadcast vectors of centeres

    husky::list_execute(
        data_list, 
        {},
        {},
        [this](DataObject<FeatureType>& data){
            auto update_func = [](vector<IdVectorPair>& collector, const IdVectorPair& e) {
                    collector.emplace_back(e);
            };

            if (this->_leftLabels.find(data.id()) != this->_leftLabels.end()) {
                this->_leftAgg.update(
                    update_func,
                    std::make_pair(data.getItemId(), data.getItemVector())
                );
            }

            if (this->_rightLabels.find(data.id()) != this->_rightLabels.end()) {
                this->_rightAgg.update(
                    update_func,
                    std::make_pair(data.getItemId(), data.getItemVector())
                );
            }
        }
    );
    husky::lib::AggregatorFactory::sync();

    // assign
    thread_local auto &pq_list =
        husky::ObjListStore::create_objlist<PQBlock>();
    _pq_list = &pq_list;

    thread_local auto& channel = 
        husky::ChannelStore::create_push_channel<int>(data_list, pq_list);
    _data2pqblock = &channel;

    auto& leftCenters = this->_leftAgg.get_value();
    auto& rightCenters = this->_rightAgg.get_value();
    husky::list_execute(
        data_list, 
        {},
        {&channel},
        [&leftCenters, &rightCenters, &channel, &distor](DataObject<FeatureType>& data){
            pair<int,int> center(-1, -1);
            float minDist = std::numeric_limits<float>::max();
            for (const IdVectorPair & p : leftCenters) {
                float curDist = distor(data.getItemVector(), p.second);
                if (curDist < minDist) {
                    center.first = p.first;
                    minDist = curDist;
                }
            }

            minDist = std::numeric_limits<float>::max();
            for (const IdVectorPair& p : rightCenters) {
                float curDist = distor(data.getItemVector(), p.second);
                if (curDist < minDist) {
                    center.second = p.first;
                    minDist = curDist;
                }
            }
            assert(center.first >= 0 && center.second >= 0);
            channel.push(data.id(), center);
        }
    );

    husky::list_execute(
        pq_list,
        {&channel},
        {},
        [&channel](PQBlock& pqblock){
            auto& msgs = channel.get(pqblock);
            pqblock._itemIds = msgs;
    });
        
    int numCenters = count(pq_list);
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "number of pq centers is " << numCenters << endl;
    }
}

}
}
