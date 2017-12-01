/*
 * Copyright 2016 Husky Team
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
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "lshcore/lshbucket.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
using namespace husky::losha;

template<
    typename ItemIdType,
    typename ItemElementType, 
    typename QueryMsg, 
    typename AnswerMsg
>
class E2LSHQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    // explicit E2LSHQuery(const typename E2LSHQuery::KeyT& id):LSHQuery(id){}
    explicit E2LSHQuery(const typename E2LSHQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id) {}
    void query(
        LSHFactory<ItemIdType, ItemElementType>& fty,
        const vector<AnswerMsg>& inMsg) override {
        this->queryMsg = this->getItemId();
        for (auto& bId : fty.calItemBuckets(this->getQuery())) {
            this->sendToBucket(bId);
        }
    }
};

template<
    typename ItemIdType,
    typename ItemElementType, 
    typename QueryMsg, 
    typename AnswerMsg
>
class E2LSHItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    // explicit E2LSHItem(const typename E2LSHItem::KeyT& id):LSHItem(id){}
    explicit E2LSHItem(const typename E2LSHItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}

    virtual void answer(
        LSHFactory<ItemIdType, ItemElementType>& factory,
        const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for (auto& queryId : inMsgs) {

            if (evaluated.find(queryId)!= evaluated.end()) continue;
            evaluated.insert(queryId);

            // get broadcasted value
            auto& queryVector = 
                factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());

            if (distance < 50){
                std::string result;
                result += std::to_string(queryId) + " ";
                result += std::to_string(this->getItemId()) + " " + std::to_string(distance) + "\n";
                
                if (husky::Context::get_param("outputPath") == "localhost"){
                    husky::LOG_I << "OUTPUT:" << result;
                }else{
                    husky::io::HDFS::Write(
                        husky::Context::get_param("hdfs_namenode"),
                        husky::Context::get_param("hdfs_namenode_port"),
                        result,
                        husky::Context::get_param("outputPath"),
                        husky::Context::get_global_tid());
                }
            }
        }
    }
};

template<
    typename ItemIdType,
    typename ItemElementType, 
    typename QueryMsg, 
    typename AnswerMsg
>
class E2LSHBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    // explicit E2LSHBucket(const typename E2LSHBucket::KeyT& bId):LSHBucket(bId){}
    explicit E2LSHBucket(const typename E2LSHBucket::KeyT& bId):LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(bId){}
};
