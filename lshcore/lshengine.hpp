/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <string>
#include <vector>
#include <functional>
#include "base/serialization.hpp"
#include "base/log.hpp"
#include "base/thread_support.hpp"
#include "boost/functional/hash.hpp"
#include "core/combiner.hpp"
#include "core/engine.hpp"
#include "io/input/line_inputformat.hpp"
#include "lib/aggregator_factory.hpp"

#include "lshbucket.hpp"
#include "lshitem.hpp"
#include "lshquery.hpp"
#include "lshstat.hpp"
#include "lshcore/loader/loader.h"

using std::vector;
using std::string;
namespace std {
template<>
class hash<vector<int>> {
public:
    size_t operator() (const vector<int> & v) const {
        size_t val = 0;
        for (auto & x : v) ::boost::hash_combine(val, x);
        return val;
    }
};
}  // namespace std

namespace husky {
namespace losha {

// assume input is set for InputFormat
template<typename BucketType, typename ItemType,
    typename ItemIdType, typename ItemElementType, typename InputFormat>
void loadItems(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    husky::ObjList<BucketType>& bucket_list,
    husky::ObjList<ItemType>& item_list,
    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
    InputFormat& infmt) {

    if (husky::Context::get_global_tid() == 0)
        husky::LOG_I << "(in loadItems) start: load items" << std::endl;

    auto& loadItemCH = 
        husky::ChannelStore::create_push_channel<
            vector<ItemElementType>>(infmt, item_list);

    auto& loadBucketCH = 
        husky::ChannelStore::create_push_channel<int>(item_list, bucket_list);

    husky::load(infmt, 
        item_loader(loadItemCH, setItem));

    // create item object, need list execute to active the object creation
    husky::list_execute(item_list, 
        [&factory, &loadItemCH, &loadBucketCH](ItemType& item) {
            auto msgs = loadItemCH.get(item);
            assert(msgs.size() == 1);

            item.setItemVector(msgs[0]);
            assert(item.getItemVector().size() != 0);
            // calculate buckets
            vector< vector<int> > myBuckets 
                = factory.calItemBuckets(item);

            // send message to create bucket object
            vector< vector<int> >::iterator it;
            for (it = myBuckets.begin(); it != myBuckets.end(); ++it) {
                loadBucketCH.push(item.getItemId(), *it);
            }
        }
    );

    husky::list_execute(bucket_list,
        [&loadBucketCH](BucketType& bucket) {
            auto& msgs = loadBucketCH.get(bucket);
            bucket.itemIds_ = msgs;
            bucket.itemIds_.shrink_to_fit();
    });

    if (husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "(in loadItems) finish: load items" << std::endl;


    // calculate number of buckets in each table
    statTableSizes(bucket_list, factory);
    return;
}

// assume inputPath is set for intputformat
template<typename QueryType,
         typename ItemIdType, typename ItemElementType, typename InputFormat >
void loadQueries(
    husky::ObjList<QueryType>& query_list,
    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
    InputFormat& infmt) {

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "in loadQueries: start to load queries" << std::endl;
    }

    auto& loadQueryCH = 
        husky::ChannelStore::create_push_channel<
            vector<ItemElementType>>(infmt, query_list);

    husky::load(infmt, 
        item_loader(loadQueryCH, setItem));

    husky::list_execute(query_list, 
        [&loadQueryCH](QueryType& query) {
            auto msgs = loadQueryCH.get(query);
            assert(msgs.size() == 1);

            query.setItemVector(msgs[0]);
            assert(query.getItemVector().size() != 0);
    });

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "in loadQueries: finish loading queries" << std::endl;
    }
}

// broadcast all queries in query_list to lshfactory
std::once_flag broadcast_flag;
template<
    typename ItemIdType, typename ItemElementType, typename QueryType>
void broadcastQueries(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    husky::ObjList<QueryType>& query_list){

    std::function<void(ItemIdType&, std::vector<ItemElementType>& ) > query_handler = 
            [&](ItemIdType& first, std::vector<ItemElementType>& second){
                factory.insertQueryVector(first, second);
            };
    broadcastQueries( query_handler, query_list);
}

template<
    typename ItemIdType, typename ItemElementType, typename QueryType>
void broadcastQueries(
    std::function<void(ItemIdType&, std::vector<ItemElementType>& ) > query_handler,
    husky::ObjList<QueryType>& query_list){
    // define query aggregator and set up _idToQueryVector in factory
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "in broadcastQueries" << std::endl;
    }
    typedef std::pair<ItemIdType, std::vector<ItemElementType>> IdVectorPair;
    typedef std::vector<IdVectorPair> QueryColType;

    auto update_to_col = [](QueryColType& collector, const IdVectorPair& e) {
        // husky::LOG_I << "broadcast out " << e.first << std::endl;
        collector.push_back(e);
    };

    husky::lib::Aggregator< QueryColType > query_vector_agg(
        QueryColType(), // parameter for initialization 
        [update_to_col](QueryColType& a, const QueryColType& b) { // parameter for aggregation rule
            for (const auto& e : b ) {
                update_to_col(a, e);
            }
        },
        [](QueryColType& col) {col.clear();}, // parameter for reset aggregator
        [update_to_col](husky::base::BinStream& in, QueryColType& col) { // parameter for deserialization
            col.clear();
            for (size_t n = husky::base::deser<size_t>(in); n--;) {
                update_to_col(col, husky::base::deser<IdVectorPair>(in));
            }
        }, 
        [](husky::base::BinStream& out, const QueryColType& col){ // parameter for serialization
            out << col.size();
            for (auto& vec : col) {
                out << vec;
            }
        });

    husky::list_execute(query_list,
        [&query_vector_agg, &update_to_col](QueryType& query) {
        query_vector_agg.update(
            update_to_col,
            std::make_pair(query.getItemId(), query.getItemVector()));
    });

    husky::lib::AggregatorFactory::sync();
    // should flush 
    // insert broadcast query to factory, call once
    std::call_once(broadcast_flag, [&query_handler, &query_vector_agg](){
        for (auto p : query_vector_agg.get_value()) {
            query_handler(p.first, p.second);
        }
    });
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finished broadcastQueries" << std::endl;
    }
}


template<
    typename QueryType, typename BucketType, typename ItemType,
    typename QueryMsg, typename AnswerMsg, 
    typename ItemIdType, typename ItemElementType, 
    typename InputFormat>
void loshaengine(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    void (*setItem)(boost::string_ref&, ItemIdType& itemId, vector<ItemElementType>&),
    InputFormat& infmt, 
    std::string itemPath,
    std::string queryPath,
    int ITERATION = 1,
    bool isQueryMode = true) {

    if (husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "start: similar items search for queries in batches\n\n" << std::endl;

    auto job_start = std::chrono::steady_clock::now();

    auto & item_list =
        husky::ObjListStore::create_objlist<ItemType>();
    auto & bucket_list = husky::ObjListStore::create_objlist<BucketType>();
    infmt.set_input(itemPath);
    loadItems(factory, bucket_list, item_list, setItem, infmt);

    auto & query_list =
        husky::ObjListStore::create_objlist<QueryType>();
    infmt.set_input(queryPath);
    loadQueries(query_list, setItem, infmt);

    // end of debug
    broadcastQueries(factory, query_list);

    if (husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "\n\nstart: similar items search for queries in batches" << std::endl;

    // define three channels
    auto& query2BucketCH = 
        husky::ChannelStore::create_push_channel<
            QueryMsg>(query_list, bucket_list);
    auto& bucket2ItemCH = 
        husky::ChannelStore::create_push_channel<
            QueryMsg>(bucket_list, item_list);
    auto& item2QueryCH = 
        husky::ChannelStore::create_push_channel<
            AnswerMsg>(item_list, query_list);

    double accumualteIterationTime = 0.0;
    for (int iter = 0; iter < ITERATION; ++iter) {
        if (husky::Context::get_global_tid() == 0) 
            husky::LOG_I << "start iteration: " + std::to_string(iter) << std::endl;

        auto time_iter_start = std::chrono::steady_clock::now();

        // execute queries
        husky::list_execute(query_list,
            [&factory, &item2QueryCH, &query2BucketCH](QueryType& query) {
                if (query.finished) return;
                auto& inMsg = item2QueryCH.get(query);
                query.query(factory, inMsg);
                for (auto& bId : QueryType::query_msg_buffer) {
                    query2BucketCH.push(query.queryMsg, bId);
                }
                QueryType::query_msg_buffer.clear();
        });

        auto time_query_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_query = time_query_finished - time_iter_start;
        if (husky::Context::get_global_tid() == 0) 
            husky::LOG_I << "iteration " + std::to_string(iter) 
               << ": finish execute query in " 
               << std::to_string(d_query.count() / 1000.0) + " seconds" << std::endl;

        // execute buckets
        husky::list_execute(bucket_list, 
            {&query2BucketCH}, {&bucket2ItemCH}, 
            [&factory, &query2BucketCH, &bucket2ItemCH](BucketType& bucket) {

                for (auto& msg : query2BucketCH.get(bucket)) {
                    // forward query, should do message reduction
                    for (auto& itemId : bucket.itemIds_) {
                        // bucket2ItemCH.push<husky::IdenCombine>(msg, itemId, item_list);
                        // worker.send_message(msg, itemId, item_list);
                        bucket2ItemCH.push(msg, itemId);
                    }
                }
        });

        auto time_bucket_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_forward = time_bucket_finished - time_query_finished;
        if (husky::Context::get_global_tid() == 0) 
            husky::LOG_I << "iteration " + std::to_string(iter) 
                << ": finish execute forward in " 
                << std::to_string(d_forward.count() / 1000.0) + " seconds" << std::endl;

        // execute Items
        husky::list_execute(item_list,
            [&factory, &bucket2ItemCH](ItemType& item) {

            const vector<QueryMsg>& inMsg = bucket2ItemCH.get(item);

            item.answer(factory, inMsg);

        });

        for (auto& pair : ItemType::item_msg_buffer) {
            item2QueryCH.push(pair.second, pair.first);
        }
        ItemType::item_msg_buffer.clear();

        for (auto& p : ItemType::topk_item_msg_buffer) {
            for (auto& msg : p.second) {
                item2QueryCH.push(msg, p.first);
            }
        }
        ItemType::topk_item_msg_buffer.clear();
        item2QueryCH.out();
        // ChannelManager out_manager(item2QueryCH);
        // out_manager.flush();

        auto time_item_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_answer = time_item_finished - time_bucket_finished;
        if (husky::Context::get_global_tid() == 0)
            husky::LOG_I << "iteration " + std::to_string(iter)
                << ": finish execute answer in "
                << std::to_string(d_answer.count() / 1000.0) + " seconds" << std::endl;

        // report per iteration time
        auto time_iter_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_iteration = time_iter_finished -time_iter_start;
        if (husky::Context::get_global_tid() == 0) 
            husky::LOG_I << "finish iteration: "
                << std::to_string(iter) 
                << " in " + std::to_string(d_iteration.count() / 1000.0) + " seconds" << std::endl << std::endl;

        // report accumulated time
        accumualteIterationTime += d_iteration.count() / 1000.0;
        if (husky::Context::get_global_tid() == 0)
            husky::LOG_I << "finish iteration: " 
                << std::to_string(iter) 
                << " and accumulate time: "
                << std::to_string(accumualteIterationTime) + " seconds" << std::endl;
    }

    auto job_finished = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_job = job_finished - job_start;
    if (husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "finished: similar items search for queries in batches in " 
            << std::to_string(d_job.count() / 1000.0) 
            << " seconds" << std::endl;
}

} // namespace losha
} // namespace husky
