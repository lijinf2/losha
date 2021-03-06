#pragma once
#include "core/engine.hpp"
#include "lib/aggregator_factory.hpp"
#include <vector>
#include <utility>
#include <functional>
using std::vector;
using std::pair;

namespace husky {
namespace losha {

template<typename ObjT>
unsigned count(
    husky::ObjList<ObjT>& obj_list) {

    husky::lib::Aggregator<unsigned> countAgg(0,
            [](unsigned& a, const unsigned& b){  a += b; });

    auto& ac = husky::lib::AggregatorFactory::get_channel();
    husky::list_execute(obj_list, {}, {&ac}, 
        [&countAgg](ObjT& obj) {
        countAgg.update(1);
     });
    husky::lib::AggregatorFactory::sync();
    unsigned size = countAgg.get_value();
    return size;
}

// key value aggregator of max, min, sum 
template<typename ObjT, typename KeyType, typename ValueType, typename CombinerType>
vector<pair<KeyType, ValueType>> keyValueCombine(
    husky::ObjList<ObjT>& obj_list,
    std::function<KeyType(const ObjT&)> getKey,
    std::function<ValueType(const ObjT&)> getValue) {

    class Pair {
    public:
        using KeyT = KeyType;
        explicit Pair(const KeyT& key) : _key(key) {}
        const KeyT& id() const { return _key; }

        KeyT _key;
        ValueType _value;

    };

    auto& agg_list = husky::ObjListStore::create_objlist<Pair>();
    auto& channel=
            husky::ChannelStore::create_push_combined_channel<ValueType, CombinerType>(obj_list, agg_list);

    husky::list_execute(
        obj_list,
        {},
        {&channel},
        [&channel, &getValue, &getKey](ObjT& obj){
        channel.push(getValue(obj), getKey(obj));
    });

    // aggregate results

    typedef vector<pair<KeyType, ValueType>> ColType;
    husky::lib::Aggregator<ColType> key_value_agg(ColType(), 
        [](ColType& a, const ColType& b){
            for (const auto& p : b) {
                a.push_back(p);
            }
        },
        [](ColType& v) {
            v = std::move( ColType());
        }
    );

    auto update_func = [](ColType& collector, const pair<KeyType, ValueType> p) {
        collector.push_back(p);
    };

    husky::list_execute(
        agg_list,
        {&channel},
        {},
        [&channel, &key_value_agg, &update_func](Pair& p){
        key_value_agg.update(
            update_func,
            std::make_pair(p.id(), channel.get(p)));
    });

    husky::lib::AggregatorFactory::sync();
    return key_value_agg.get_value();
}

template<typename ObjT, typename KeyType, typename ValueType>
pair<KeyType, ValueType> keyValueAggMax(
    husky::ObjList<ObjT>& obj_list,
    std::function<pair<KeyType, ValueType>(const ObjT&)> getKeyValue) {

    husky::lib::Aggregator<pair<KeyType, ValueType>> maxAgg(
        std::make_pair(0, std::numeric_limits<ValueType>::min()), 
        [](pair<KeyType, ValueType>& a, const pair<KeyType, ValueType>& b){ 
            if (a.second < b.second) a = b; },
        [](pair<KeyType, ValueType>& p){
            p = std::make_pair(0, std::numeric_limits<ValueType>::min());
        });

    auto& maxCh = husky::lib::AggregatorFactory::get_channel();
    husky::list_execute(
        obj_list, {}, {&maxCh}, [&maxAgg, &getKeyValue](ObjT& obj) {
        maxAgg.update(getKeyValue(obj));
    });
    return maxAgg.get_value();
}

template<typename ObjT, typename KeyType, typename ValueType>
pair<KeyType, ValueType> keyValueAggMin(
    husky::ObjList<ObjT>& obj_list,
    std::function<pair<KeyType, ValueType>(const ObjT&)> getKeyValue) {

    husky::lib::Aggregator<pair<KeyType, ValueType>> minAgg(
        pair<KeyType, ValueType>(-1, std::numeric_limits<ValueType>::max()), 
        [](pair<KeyType, ValueType>& a, const pair<KeyType, ValueType>& b){ 
            if (a.second > b.second) a = b;
    },  [](pair<KeyType, ValueType>& p) {
            p = pair<KeyType, ValueType>(-1, std::numeric_limits<ValueType>::max());
    });

    auto& ch = husky::lib::AggregatorFactory::get_channel();
    husky::list_execute(
        obj_list, {}, {&ch}, [&minAgg, &getKeyValue](ObjT& obj) {
        auto p = getKeyValue(obj);
        minAgg.update(p);
    });
    return minAgg.get_value();
}

template<typename ObjT, typename KeyType, typename ValueType>
vector<pair<KeyType, ValueType>> keyValueAggTopK(
    husky::ObjList<ObjT>& obj_list,
    std::function<pair<KeyType, ValueType>(const ObjT&)> getKeyValue,
    int K) {
    auto comparator = [](const pair<KeyType, ValueType>& p1, const pair<KeyType, ValueType>& p2){
        return p1.second > p2.second;
    };

    auto add_to_topk = [K, &comparator](vector<pair<KeyType, ValueType>>& pairs, const pair<KeyType, ValueType>& p) {
        pairs.emplace_back(p);
        std::push_heap(
            pairs.begin(),
            pairs.end(),
            comparator);
        if (pairs.size() > K) {
            std::pop_heap(pairs.begin(), pairs.end(), comparator);
            pairs.pop_back();
        }
    };

    husky::lib::Aggregator<vector<pair<KeyType, ValueType>>> topkAgg(
        vector<pair<KeyType, ValueType>>(),
        [&add_to_topk](vector<pair<KeyType, ValueType>>& a, const vector<pair<KeyType, ValueType>>& b) {
            for (auto& i : b)
                add_to_topk(a, i);
        },
        [](vector<pair<KeyType, ValueType>>& a) { a.clear(); }
    );

    husky::list_execute(
        obj_list,
        [&topkAgg, &add_to_topk, &getKeyValue](ObjT& obj){
            auto p = getKeyValue(obj);
            topkAgg.update(add_to_topk, p);
        }
    );

    husky::lib::AggregatorFactory::sync();
    return topkAgg.get_value();
}

template<typename ObjT, typename ValueType>
ValueType sumAgg (
    husky::ObjList<ObjT>& obj_list,
    std::function<ValueType(ObjT&)> getValue) {
    husky::lib::Aggregator<ValueType> sumAgg(0, [](ValueType& a, const ValueType& b) { a += b; });
    husky::list_execute(
        obj_list, 
        {},
        {},
        [&sumAgg, &getValue](ObjT& obj){

        sumAgg.update(getValue(obj));
    });
    husky::lib::AggregatorFactory::sync();
    return sumAgg.get_value();
} 

template<typename ValueType>
vector<husky::lib::Aggregator<vector<ValueType>>> vectorAggs( int numAggs = 1) { 

    assert(numAggs > 0);
    typedef vector<ValueType> AggType;
    vector<husky::lib::Aggregator<AggType>> aggs;
    aggs.reserve(numAggs);
    for (int i = 0; i < numAggs; ++i) {
        aggs.emplace_back(
            husky::lib::Aggregator<AggType>(
                AggType(),
                [](AggType& a, const AggType& b){
                    for (const auto& e : b) {
                        a.emplace_back(e);
                    }
                },
                [](AggType& col){
                    col.clear();
                })
        );
    }
    return aggs; 
}

}
}
