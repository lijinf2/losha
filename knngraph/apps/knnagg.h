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


}
}
