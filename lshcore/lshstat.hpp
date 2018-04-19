#pragma once
#include <iostream>
#include <utility>
#include <climits>
#include "core/engine.hpp"
#include "lib/aggregator_factory.hpp"

#include "lshcore/lshfactory.hpp"

namespace husky {
namespace losha {

template<typename BucketType, typename ItemIdType, typename ItemElementType>
void statTableSizes(
    husky::ObjList<BucketType>& bucket_list,
    LSHFactory<ItemIdType, ItemElementType>& factory) {

    // get maximum tableId
    husky::lib::Aggregator<unsigned> maxAgg(0,
        [](unsigned& a, const unsigned& b){ if (a < b) a = b; });

    auto& ac = husky::lib::AggregatorFactory::get_channel();
    husky::list_execute(bucket_list,{}, {&ac}, 
        [&maxAgg](BucketType& bucket) {
        maxAgg.update(bucket.getTable());
    });
    husky::lib::AggregatorFactory::sync();
    unsigned maxTableIndex = maxAgg.get_value();
    unsigned numTables = maxTableIndex + 1;

    // aggregate table size
    husky::lib::Aggregator<vector<unsigned>> num_buckets_agg(vector<unsigned>(numTables), 
        [](vector<unsigned>& a, const vector<unsigned>& b){
            for (int i = 0; i < a.size(); ++i) {
                a[i] += b[i];
            }
        },
        [numTables](vector<unsigned>& v) {
            v = std::move(vector<unsigned>(numTables, 0));
        }
    );

    auto update_func = [](vector<unsigned>& collector, unsigned tableId) {
        collector[tableId]++;;
    };

    husky::list_execute(bucket_list,
        [&num_buckets_agg, &update_func](BucketType& bucket) {
        num_buckets_agg.update(
            update_func,
            bucket.getTable());
    });

    husky::lib::AggregatorFactory::sync();
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "total number of tables is : " << numTables << std::endl;
        husky::LOG_I << "now report sizes of every table in (table Idx, numBuckets)" << std::endl;
        const auto& tableSizes = num_buckets_agg.get_value();
        for (int i = 0; i < tableSizes.size(); ++i) {
            husky::LOG_I << "table " << i << " has " << tableSizes[i] << " buckets " << std::endl;
        }
        husky::LOG_I << std::endl;
    }
}
}
}
