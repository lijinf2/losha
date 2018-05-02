#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "core/engine.hpp"
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

    template<typename ItemElementType>
    static void train(
        husky::ObjList<AdjObject>& adj_list,
        husky::ObjList<DataObject<ItemElementType>>& data_list);
    
private:
};

template<typename ItemElementType>
void Block::train(
    husky::ObjList<AdjObject>& adj_list,
    husky::ObjList<DataObject<ItemElementType>>& data_list) {

    // build block_list, block_id = label
    auto & block_list =
        husky::ObjListStore::create_objlist<Block>();
    auto& load_channel = 
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
            blk._records = load_channel.get(blk);
        });
    // get item and local join
    auto& request_channel = 
        husky::ChannelStore::create_push_channel<int>(block_list, data_list);
    husky::list_execute(
        block_list,
        {},
        {&request_channel},
        [&request_channel](Block& blk) {
            for (const auto& record : blk._records) {
                for (auto itemId : record._nbs) {
                    request_channel.push(blk.id(), itemId);
                }
            }
        });
    auto& response_channel = 
        husky::ChannelStore::create_push_channel<DenseVector<int, ItemElementType>>(data_list, block_list);

    husky::list_execute(
        data_list,
        {&request_channel},
        {&response_channel},
        [&request_channel, &response_channel](DataObject<ItemElementType>& data) {
            const auto& msgs = request_channel.get(data);
            unordered_set<int> visited;
            for (auto bid : msgs) {
                if (visited.find(bid) == visited.end()) { 
                    visited.insert(bid);
                    response_channel.push(data.getItem(), bid);
                }
            }
        });

    //@ avoid three copies of the dataset, probably write disk and then read disk
    husky::list_execute(
        block_list,
        {},
        {},
        [](Block& blk){
            unordered_map<int, const vector<ItemElementType>*> map;
        });
}
}
}
