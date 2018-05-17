#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "base/log.hpp"
#include "core/engine.hpp"

#include "losha/common/distor.hpp"
#include "adjobject.h"
#include "dataobject.h"
using std::vector;
using std::unordered_set;
using std::unordered_map;

namespace husky {
namespace losha {

class AdjRecord {
public:
    int _vid;
    vector<int> _nbs;
    AdjRecord(){};
    AdjRecord(int id, vector<int> nbs) {
        _vid = id;
        _nbs.swap(nbs);
    }
    husky::BinStream& serialize(husky::BinStream& stream) const {
        stream << _vid << _nbs;
        return stream;
    }

    husky::BinStream& deserialize(husky::BinStream& stream) {
        stream >> _vid >> _nbs;
        return stream;
    }

};

class Block{
public:
    using KeyT = int;
    KeyT _bid;
    explicit Block(const KeyT& id) : _bid(id) {};
    const KeyT id() const {return _bid;};

    vector<AdjRecord> _records;

    unordered_set<int> calRequestDataId() const { 
        unordered_set<int> requested;
        for (const auto& record : _records) {
            for (auto itemId : record._nbs) {
                if (requested.find(itemId) == requested.end())
                    requested.insert(itemId);
            }
        }
        return requested;
    }
    template<typename ItemElementType>
    static void train(
        husky::ObjList<AdjObject>& adj_list,
        husky::ObjList<DataObject<ItemElementType>>& data_list);

private:
    static void reportBlockAndNumRequestItems(
        husky::ObjList<Block>& block_list) {
        // debug information
        vector<pair<int, int>> agg =
            keyValueCombine<Block, int, int, husky::SumCombiner<int>>(
                block_list,
                [](const Block& blk){ return blk.id();},
                [](const Block& blk){ return blk.calRequestDataId().size();});
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "report (block_id, num_request_items)" << std::endl;
            std::sort(
                agg.begin(),
                agg.end(),
                [](const pair<int, int>& a, const pair<int, int>&b){
                    return a.second > b.second;
                });
            for (int i = 0; i < agg.size(); ++i) {
                husky::LOG_I << "(" << agg[i].first << ", " << agg[i].second << ")" << std::endl;
            }
        }
    }

    static unsigned getNumTotalRequestItems(
        husky::ObjList<Block>& block_list) {
        husky::lib::Aggregator<unsigned> sum(
            0,
            [](unsigned &a, const unsigned &b) { a = a + b;});

        auto& ac = husky::lib::AggregatorFactory::get_channel();
        husky::list_execute(
            block_list,
            {},
            {&ac},
            [&sum](Block& blk){
            sum.update(blk.calRequestDataId().size());
        });

        return sum.get_value();
    }
};

template<typename ItemElementType>
void Block::train(
    husky::ObjList<AdjObject>& adj_list,
    husky::ObjList<DataObject<ItemElementType>>& data_list) {

    // build block_list, block_id = label
    thread_local auto & block_list =
        husky::ObjListStore::create_objlist<Block>();
    thread_local auto& load_channel = 
        husky::ChannelStore::create_push_channel<
            AdjRecord>(adj_list, block_list);
    husky::list_execute(
        adj_list,
        {},
        {&load_channel},
        [&load_channel](AdjObject& adj){
            AdjRecord record(adj.id(), adj.getNB());
            load_channel.push(
                record,
                adj._label);
        });

    husky::list_execute(
        block_list,
        {&load_channel},
        {},
        [&load_channel](Block& blk) {
            const auto& msgs = load_channel.get(blk); 
            blk._records = msgs;
        });

    // every thread should execute getNumTotalRequestItems
    int numTotalRequests = getNumTotalRequestItems(block_list);
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "Total number of requests is " << numTotalRequests << std::endl;
    }
    // get item and local join
    thread_local auto& request_channel = 
        husky::ChannelStore::create_push_channel<int>(block_list, data_list);
    
    husky::list_execute(
        block_list,
        {},
        {&request_channel},
        [&request_channel](Block& blk) {
            auto requested = blk.calRequestDataId();
            for (auto itemId : requested) {
                request_channel.push(blk.id(), itemId);
            }
        });

    typedef DenseVector<int, ItemElementType> DV;
    thread_local auto& response_channel = 
        husky::ChannelStore::create_push_channel<DV>(data_list, block_list);

    husky::list_execute(
        data_list,
        {&request_channel},
        {&response_channel},
        [&request_channel, &response_channel](DataObject<ItemElementType>& data) {
            const auto& msgs = request_channel.get(data);
            // unordered_set<int> visited;
            for (auto bid : msgs) {
                // if (visited.find(bid) == visited.end()) { 
                //     visited.insert(bid);
                    response_channel.push(data.getItem(), bid);
                // }
            }
        });

    //@ avoid three copies of the dataset, probably write disk and then read disk
    thread_local auto& result_channel = 
        husky::ChannelStore::create_push_channel<pair<int, float>>(block_list, adj_list);
    husky::list_execute(
        block_list,
        {&response_channel},
        {&result_channel},
        [&response_channel, &result_channel](Block& blk){
            // build mapping
            const auto& msgs = response_channel.get(blk);
            unordered_map<int, const vector<ItemElementType>*> idToVector;
            for (const DV& data : msgs) {
                idToVector[data.getItemId()] = &(data.getItemVector());
            }
            // all pair training inside each record
            pair<int, float> sentMsg;
            for (const AdjRecord& record : blk._records) {
                for (int i = 0; i < record._nbs.size(); ++i) {
                    int leftId = record._nbs[i];

                    for (int j = i + 1; j < record._nbs.size(); ++j) {
                        int rightId = record._nbs[j];

                        float dist = calE2Dist(*(idToVector[leftId]), *(idToVector[rightId]));

                        sentMsg = std::make_pair(rightId, dist);
                        result_channel.push(sentMsg, leftId);

                        sentMsg = std::make_pair(leftId, dist);
                        result_channel.push(sentMsg, rightId);
                    }
                }
            }
        });

    // update adj_list by result_channel
    husky::list_execute(
        adj_list,
        {&result_channel},
        {},
        [&result_channel](AdjObject& adj){
        const vector<pair<int, float>>& msgs = result_channel.get(adj);
        if (msgs.size() > 0)
            adj.mergeFoundKNN(msgs);
    });

    AdjObject::updateRKNN(adj_list);
}

}
}
