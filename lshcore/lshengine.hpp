/*
 * Copyright 2016 husky::Team
 *
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

#include "boost/functional/hash.hpp"
#include "core/combiner.hpp"
#include "base/log.hpp"
#include "core/engine.hpp"
#include "base/thread_support.hpp"
#include "io/input/line_inputformat.hpp"

#include "lshbucket.hpp"
#include "lshitem.hpp"
#include "lshquery.hpp"

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

template<typename ItemType, typename ItemIdType, typename ItemElementType>
auto item_loader(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    husky::PushChannel< vector<ItemElementType>, ItemType > &ch,
    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&)) {

    auto parse_lambda = [&factory, &ch, &setItem]
    (boost::string_ref & line) {
        try {
            ItemIdType itemId;
            vector<ItemElementType> itemVector;
            setItem(line, itemId, itemVector);

            ch.push(itemVector, itemId);
        } catch(std::exception e) {
            assert("bucket_parser error");
        }
    };
    return parse_lambda;
}

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
        husky::base::log_msg("(in loadItems) start: load items");

    auto& loadItemCH = 
        husky::ChannelStore::create_push_channel<
            vector<ItemElementType>>(infmt, item_list);

    auto& loadBucketCH = 
        husky::ChannelStore::create_push_channel<int>(item_list, bucket_list);

    husky::load(infmt, 
        item_loader(factory, loadItemCH, setItem));

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
        husky::base::log_msg("(in loadItems) finish: load items");
    return;
}

// assume inputPath is set for intputformat
template<typename QueryType,
         typename ItemIdType, typename ItemElementType, typename InputFormat >
void loadQueries(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    husky::ObjList<QueryType>& query_list,
    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
    InputFormat& infmt) {

    /* drop queries if there are no bucket receiver*/

    if (husky::Context::get_global_tid() == 0) {
        husky::base::log_msg("in loadQueries: start to load queries");
    }

    auto& loadQueryCH = 
        husky::ChannelStore::create_push_channel<
            vector<ItemElementType>>(infmt, query_list);

    husky::load(infmt, 
        item_loader(factory, loadQueryCH, setItem));

    // bool isBroadCastQueryOn = false;
    // if (factory.getParamExistence("broadcast"))
    //     if (factory.getParamStr("broadcast") == "1")
    //         isBroadCastQueryOn = true;;
    // if (husky::Context::get_global_tid().id == 0) {
    //     husky::base::log_msg("broadcast query: " + std::to_string(isBroadCastQueryOn));
    // }
    // message buffer can be directly cleared. Program can not work without clear message buffers.
    
    husky::list_execute(query_list, 
        [&factory, &loadQueryCH](QueryType& query) {
            auto msgs = loadQueryCH.get(query);
            assert(msgs.size() == 1);

            query.setItemVector(msgs[0]);
            assert(query.getItemVector().size() != 0);
    });

    if (husky::Context::get_global_tid() == 0) {
        husky::base::log_msg("in loadQueries: finish loading queries");
    }
}
//
// template<typename ElemT>
// struct HashCat : public husky::HashCombineBase {
//     static void combine(vector<ElemT> & val, vector<ElemT> & inc) {
//         // val.insert(val.end(), inc.begin(), inc.end());
//         val.push_back(inc[0]);
//     }
// };
//
// // this is used in experiments glove and tweets, but will cause load imbalancing
// // i.e. all the elements go to worker 0
// // namespace std {
// // template<>
// // class hash<vector<int>> {
// // public:
// //     size_t operator() (const vector<int> & v) const {
// //         size_t val = 0;
// //         for (auto & x : v) val^=x;
// //         return val;
// //         // return v[1];
// //     }
// // }
// // }
// template<typename ItemIdType, typename ItemElementType,
//          typename BucketType, typename ItemType, typename InputFormat>
// void addItems(
//     LSHFactory<ItemIdType, ItemElementType>& factory,
//     std::string itemPath,
//     void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
//     husky::ObjList<BucketType>& bucket_list,
//     husky::ObjList<ItemType>& item_list,
//     InputFormat& infmt) {
//
//     if (husky::Context::get_global_tid().id == 0) husky::base::log_msg("(in addItems) start: add items");
//     // husky::LineInputFormat<husky::HDFSFileSplitter> infmt;
//     infmt.set_input(itemPath);
//     husky::Context::get_global_tid().load(
//         infmt,
//         item_loader(factory, setItem, item_list));
//
//     husky::Context::get_global_tid().register_msg_ctor< vector<ItemElementType> >(
//         item_list,
//     [](vector<ItemElementType> &msg, const ItemIdType& key) {
//         ItemType item;
//         item.itemId = key;
//         item.itemVector.swap(msg);
//         return item;
//     });
//
//     if (husky::Context::get_global_tid().id == 0) husky::base::log_msg("finish: register new item messages and new item objects created!");
//     husky::Context::get_global_tid().list_execute(item_list, [&factory, &bucket_list](ItemType& item) {
//         // otherwise duplicate item, this two lines can be deleted
//         // should be commented
//         // avoid items loaded in the last iteration sends messages in this iteration
//         auto& msgs = husky::get_messages< vector<ItemElementType> >(item);
//         if (msgs.size() < 1) return;
//         assert(msgs.size() == 1);
//
//         vector< vector<int> > myBuckets = factory.calItemBuckets(item);
//
//         // send message to create bucket object
//         vector< vector<int> >::iterator it;
//         for (it = myBuckets.begin(); it != myBuckets.end(); ++it) {
//             // In case we need to save memory
//             // vector<ItemIdType> msg {item.getItemId()}
//             // vector<ItemIdType> msg;
//             // husky::Context::get_global_tid().send_message<HashCat<ItemIdType>>(msg, *it, bucket_list);
//
//             husky::Context::get_global_tid().send_message(item.getItemId(), *it, bucket_list);
//         }
//     });
//
//     if (husky::Context::get_global_tid().id == 0) husky::base::log_msg("finish: register buckets messages and bucket objects created!");
//     // create bukcet object, need list execute to activate the object creation
//     husky::Context::get_global_tid().register_msg_ctor<ItemIdType>(
//         bucket_list,
//     [] (const ItemIdType &itemId, const vector<int> & key) {
//         BucketType b;
//         // b.bucketId.swap(key);
//         b.bucketId = key;
//         // b.bucketId = std::move(key);
//         return b;
//     });
//
//     // if not enable aggregator
//     if (!LSHContext::isCollectorOn()) {
//         // query mode: mapping same as batch mode
//         husky::Context::get_global_tid().list_execute(bucket_list,
//         [&factory](BucketType& bucket) {
//             // push items into bucket
//             auto& msgs = husky::get_messages< ItemIdType >(bucket);
//             for (auto& m : msgs)
//                 bucket.itemIds.push_back(m);
//
//             // //debug
//             // std::string debugStr = "";
//             // debugStr += "bucket Id = " + std::to_string(bucket.id());
//             // debugStr += ", and # of items =  " + std::to_string(bucket.itemIds.size());
//             // debugStr += "\n";
//             // bucket.write_to_hdfs(debugStr, factory.getParamStr("outputPath"));
//         });
//
//         husky::Context::get_global_tid().list_execute(item_list, [&](ItemType& item) {
//             husky::get_messages<ItemIdType>(item);
//         });
//         if (husky::Context::get_global_tid().id == 1) husky::base::log_msg("(in addItems) finish: add items");
//         return;
//     }
//
//     if (husky::Context::get_global_tid().id == 1) husky::base::log_msg("(in loadItems) finish: load items");
//     return;
// }
//
// first build item list, then bucket list

//
// template<typename ItemIdType, typename ItemElementType,
//          typename QueryType, typename InputFormat >
// void addQueries(
//     LSHFactory<ItemIdType, ItemElementType>& factory,
//     std::string queryPath,
//     void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
//     husky::ObjList<QueryType>& query_list,
//     InputFormat& infmt) {
//
//     /* drop queries if there are no bucket receiver*/
//
//     if (husky::Context::get_global_tid().id == 0) {
//         husky::base::log_msg("in addQueries: start to add queries in " + queryPath);
//     }
//     infmt.set_input(queryPath);
//     husky::Context::get_global_tid().load(
//         infmt,
//         item_loader(factory, setItem, query_list));
//
//     // should work: why not directly create object
//     husky::Context::get_global_tid().register_msg_ctor<vector<ItemElementType>>(
//     query_list, [] (const vector<ItemElementType>& msg, const ItemIdType& key) {
//         QueryType q;
//         q.itemId = key;
//         q.itemVector = msg;
//         return q;
//     });
//
//     // bool isBroadCastQueryOn = false;
//     // if (factory.getParamExistence("broadcast"))
//     //     if (factory.getParamStr("broadcast") == "1")
//     //         isBroadCastQueryOn = true;;
//     // if (husky::Context::get_global_tid().id == 0) {
//     //     husky::base::log_msg("broadcast query: " + std::to_string(isBroadCastQueryOn));
//     // }
//     // message buffer can be directly cleared. Program can not work without clear message buffers.
//     husky::Context::get_global_tid().list_execute(query_list, [&](QueryType & query) {
//     });
//
//     if (husky::Context::get_global_tid().id == 0) {
//         husky::base::log_msg("in addQueries: finish adding queries");
//     }
// }
//

template<
    typename QueryType, typename BucketType, typename ItemType,
    typename QueryMsg, typename AnswerMsg, 
    typename ItemIdType, typename ItemElementType, 
    typename InputFormat>
void loshaengine(
    LSHFactory<ItemIdType, ItemElementType>& factory,
    void (*setItem)(boost::string_ref&, ItemIdType& itemId, vector<ItemElementType>&),
    InputFormat& infmt, bool isQueryMode = true) {

    if (husky::Context::get_global_tid() == 0) 
        husky::base::log_msg("start: similar items search for queries in batches\n\n");

    auto job_start = std::chrono::steady_clock::now();

    auto & item_list =
        husky::ObjListStore::create_objlist<ItemType>();
    auto & bucket_list = husky::ObjListStore::create_objlist<BucketType>();
    infmt.set_input(husky::Context::get_param("itemPath"));
    loadItems(factory, bucket_list, item_list, setItem, infmt);

    auto & query_list =
        husky::ObjListStore::create_objlist<QueryType>();
    infmt.set_input(husky::Context::get_param("queryPath"));
    loadQueries(factory, query_list, setItem, infmt);

    if (husky::Context::get_global_tid() == 0) 
        husky::base::log_msg("\n\nstart: similar items search for queries in batches");

    // ITERATION
    int ITERATION = std::stoi(husky::Context::get_param("iters"));

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
            husky::base::log_msg("start iteration: " + std::to_string(iter));

        auto time_iter_start = std::chrono::steady_clock::now();

        // execute queries
        husky::list_execute(query_list,
            [&factory, &item2QueryCH, &query2BucketCH](QueryType& query) {

                auto& inMsg = item2QueryCH.get(query);
                query.query(factory, inMsg);

                for (auto& bId : query.query_msg_buffer) {
                    query2BucketCH.push(query.queryMsg, bId);
                }
                query.query_msg_buffer.clear();
        });

        auto time_query_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_query = time_query_finished - time_iter_start;
        if (husky::Context::get_global_tid() == 0) 
            husky::base::log_msg(
               "iteration " + std::to_string(iter) 
               + ": finish execute query in " 
               + std::to_string(d_query.count() / 1000.0) + " seconds");

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
            husky::base::log_msg(
               "iteration " + std::to_string(iter) 
               + ": finish execute forward in " 
               + std::to_string(d_forward.count() / 1000.0) + " seconds");

        // execute Items
        husky::list_execute(item_list,
            [&factory, &bucket2ItemCH, &item2QueryCH](ItemType& item) {

                const vector<QueryMsg>& inMsg = bucket2ItemCH.get(item);
            if (inMsg.size() == 0) return;

            item.answer(factory, inMsg);

            for (auto& pair : ItemType::item_msg_buffer) {
                item2QueryCH.push(pair.second, pair.first);
            }
            ItemType::item_msg_buffer.clear();
        });

        auto time_item_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_answer = time_item_finished - time_bucket_finished;
        if (husky::Context::get_global_tid() == 0)
            husky::base::log_msg(
                "iteration " + std::to_string(iter)
                + ": finish execute answer in "
                + std::to_string(d_answer.count() / 1000.0) + " seconds");

        // report per iteration time
        auto time_iter_finished = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> d_iteration = time_iter_finished -time_iter_start;
        if (husky::Context::get_global_tid() == 0) 
            husky::base::log_msg("finish iteration: "
                + std::to_string(iter) 
                + " in " + std::to_string(d_iteration.count() / 1000.0) + " seconds");

        // report accumulated time
        accumualteIterationTime += d_iteration.count() / 1000.0;
        if (husky::Context::get_global_tid() == 0)
            husky::base::log_msg("finish iteration: " 
                + std::to_string(iter) 
                + " and accumulate time: "
                + std::to_string(accumualteIterationTime) + " seconds");
    }

    auto job_finished = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_job = job_finished - job_start;
    if (husky::Context::get_global_tid() == 0) 
        husky::base::log_msg("finished: similar items search for queries in batches in " 
            + std::to_string(d_job.count() / 1000.0) 
            + " seconds");
}

} // namespace losha
} // namespace husky
