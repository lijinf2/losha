//
// Created by darxan on 2018/4/24.
//
#pragma once
#ifndef LOSHA_LINEARSCAN_H
#define LOSHA_LINEARSCAN_H

#include "base/serialization.hpp"
#include "base/log.hpp"
#include "base/thread_support.hpp"
#include "boost/functional/hash.hpp"
#include "core/combiner.hpp"
#include "core/engine.hpp"
#include "io/input/line_inputformat.hpp"
#include "lib/aggregator_factory.hpp"

#include "lshcore/lshbucket.hpp"
#include "lshcore/lshitem.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshstat.hpp"

#include "losha/common/distor.hpp"


namespace hushk {
    namespace losha {
        std::once_flag broadcast_flag;
        template <typename QueryType, typename ItemType,typename ItemIdType, typename ItemElementType>
        class LinearScanner {

            typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
            typedef std::priority_queue<std::pair<ItemIdType, ItemElementType> > TopKQueueType;

        public:
            void linearScan() {
                int dimension = std::stoi(husky::Context::get_param("dimension"));
                int topK = std::stoi(husky::Context::get_param("topK"));
                std::string itemPath = husky::Context::get_param("itemPath");
                std::string queryPath = husky::Context::get_param("queryPath");

                int sizeOfVector = dimension * 4 + 8;

                auto& itemFormat = husky::io::InputFormatStore::create_chunk_inputformat(sizeOfVector);
                auto& queryFormat = husky::io::InputFormatStore::create_chunk_inputformat(sizeOfVector);

                itemFormat.set_input(itemPath);
                queryFormat.set_input(queryPath);

                auto& item_list  = husky::ObjListStore::create_objlist<ItemType>();
                auto& query_list = husky::ObjListStore::create_objlist<QueryType>();

                loadData (item_list, parseIdFvecs, itemFormat);
                loadData (query_list, parseIdFvecs, queryFormat);

                // broadcast queries
                // item receive queries then calculate distance
                // item return distance to query
                // retrieve top K result

                broadcastQueries(query_list);


                auto& item2queryChannel = husky::ChannelStore::create_push_channel<
                        AnswerMsg>(item_list, query_list);

                husky::list_execute(
                        item_list,
                        [&](ItemType& item) {
                            for (auto iter = _idToQueryVector.begin(); iter!=_idToQueryVector.end(); iter++) {
                                ItemElementType item_query_distance = calE2Dist(item.getItemVector(), iter->second);
                                item2queryChannel.push(std::make_pair(iter->first, item_query_distance), iter->first);
                            }
                        }
                );

                husky::list_execute(
                        query_list,
                        [&](QueryType& query) {
                            for (auto& msg : item2queryChannel.get(query)) {
                                ItemIdType itemId = msg.first;
                                ItemElementType item_query_distance = msg.second;

                                TopKQueueType& topKQueue = topKQueues[query.id()];
                                if (topKQueue.size() < topK ) {
                                
                                    topKQueue.push(std::make_pair(itemId, item_query_distance));
                                } else if (topKQueue.top().second > item_query_distance) {
                                    topKQueue.pop();
                                    topKQueue.push(std::make_pair(itemId, item_query_distance));
                                }
                            }
                        }
                );

                for (auto iter = topKQueues.begin(); iter != topKQueues.end(); ++iter) {
                    ItemIdType queryId = iter->first;
                    TopKQueueType& topKRecords = iter->second;
                    while(topKRecords.size()) {
                        const std::pair<ItemIdType, ItemElementType>& topKRecord = topKRecords.top();
                        writeHDFSTriplet(queryId, topKRecord.first, topKRecord.second, "hdfs_namenode", "hdfs_namenode_port", "outputPath");
                        topKRecords.pop();
                    }
                }

            };


        private:

            std::unordered_map<ItemIdType, std::vector<ItemElementType> > _idToQueryVector;

            std::unordered_map<ItemIdType, TopKQueueType > topKQueues;

            // Fetch broadcasted queries into _idToQueryVector
            inline void insertQueryVector(int qid, const std::vector<ItemElementType>& qvec) {
                if (_idToQueryVector.find(qid) != _idToQueryVector.end()) {
                    //ASSERT_MSG(0, "query already exists");
                }
                _idToQueryVector[qid] = qvec;
            }

            template <typename DataType,typename InputFormat >
            void loadData(
                    husky::ObjList<DataType>& data_list,
                    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&),
                    InputFormat& infmt) {

                auto& load_data_channel = husky::ChannelStore::create_push_channel<
                                          vector<ItemElementType> >(infmt, data_list);
                husky::load(
                        infmt,
                        [&load_data_channel, &setItem](boost::string_ref & line) {
                            try {
                                ItemIdType itemId;
                                vector<ItemElementType> itemVector;
                                setItem(line, itemId, itemVector);

                                load_data_channel.push(itemVector, itemId);

                            } catch(std::exception e) {
                                assert("bucket_parser error");
                            }
                        }
                );

                husky::list_execute(
                        data_list,
                        [&load_data_channel](DataType& data) {
                            auto msgs = load_data_channel.get(data);
                            assert(msgs.size()==1);

                            data.setItemVector(msgs[0]);
                            assert(data.getItemVector().size() != 0);
                        }
                );
            }

            template<
                    typename DataType>
            void broadcastQueries(husky::ObjList<DataType>& query_list) {
                typedef std::pair<ItemIdType, std::vector<ItemElementType>> IdVectorPair;
                typedef std::vector<IdVectorPair> QueryColType;

                auto update_to_col = [](QueryColType& collector, const IdVectorPair& e) {
                    collector.push_back(e);
                };

                husky::lib::Aggregator< QueryColType > query_vector_agg(
                        QueryColType(), // parameter for initialization
                        [update_to_col](QueryColType& a, const QueryColType& b) { // parameter for aggregation rule
                            for (const auto& e : b ) {
                                update_to_col(a, e);
                            }
                        },
                        [](QueryColType& col) {col.clear();},
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


                std::call_once(broadcast_flag, [&]() {
                    for (auto p : query_vector_agg.get_value()) {
                        insertQueryVector(p.first, p.second);
                    }
                });

            }

        };
    }
};

#endif //LOSHA_LINEARSCAN_H
