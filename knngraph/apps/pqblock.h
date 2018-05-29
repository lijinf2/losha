#pragma once
#include <utility>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <limits>

#include "core/engine.hpp"

#include "losha/common/aggre.hpp"

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

    template<typename FeatureType>
    static void PQ(
        husky::ObjList<DataObject<FeatureType>>& data_list, 
        husky::ObjList<PQBlock>& pq_list,
        std::function<float (const vector<FeatureType>& a, const vector<FeatureType>& b)> distor,
        int numData);
};

template<typename FeatureType>
class PQBlockAgg {
public:
    PQBlockAgg(
            husky::ObjList<DataObject<FeatureType>>& data_list,
            int numData) {
        int codebookSize = sqrt(numData);
        _leftLabels = sampleRand(numData, codebookSize, numData);
        _rightLabels = sampleRand(numData, codebookSize, numData + 1);

        // broadcast vectors of centeres
        typedef std::pair<int, std::vector<FeatureType>> IdVectorPair;
        _leftAgg = vectorAggs<IdVectorPair>(1)[0];
        _rightAgg = vectorAggs<IdVectorPair>(1)[0];

        husky::list_execute(
            data_list, 
            {},
            {},
            [this](DataObject<FeatureType>& data){
                auto update_func = [](vector<IdVectorPair>& collector, const IdVectorPair& e) {
                        collector.push_back(e);
                }

                if (this->leftLabels.find(data.id()) != this->leftLabels.end()) {
                    this->leftAgg.update(
                        update_func,
                        std::make_pair(data.getItemId(), data.getItemVector())
                    );
                }

                if (this->rightLabels.find(data.id()) != this->rightLabels.end()) {
                    this->rightAgg.update(
                        update_func,
                        std::make_pair(data.getItemId(), data.getItemVector())
                    );
                }
            }
        );
        husky::lib::AggregatorFactory::sync();

        // assign
        auto &pq_list =
            husky::ObjListStore::create_objlist<PQBlock>();

        thread_local auto& channel = 
            husky::ChannelStore::create_push_channel<int>(data_list, pq_list);

        husky::list_execute(
            data_list, 
            {},
            {&channel},
            [this, &channel](DataObject<FeatureType>& data){
                pair<int,int> center(-1, -1);
                float minDist = std::numeric_limits<float>::max();
                for (const auto& IdVectorPair p : this->_leftAgg.get_value()) {
                    float curDist = distor(data.getItemVector(), p.second);
                    if (curDist < minDist) {
                        center.first = p.first;
                        minDist = curDist;
                    }
                }

                minDist = std::numeric_limits<float>::max();
                for (const auto& IdVectorPair p : this->_rightAgg.get_value()) {
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

        husky::list_exeucte(
            pq_list,
            {&channel},
            {},
            [&channel](PQBlock& pqblock){
                pqblock._itemIds = channel.get(pqblock);
        });
            
        _pq_list = &pq_list;
    }

protected:
    unordered_set<unsigned> _leftLabels;
    husky::lib::Aggregator<vector<RecordT>> leftAgg;
    unordered_set<unsigned> _rightLabels;
    husky::lib::Aggregator<vector<RecordT>> rightAgg;

    huksy::ObjList<PQBlock>* _pq_list = nullptr;
};
}
}
