/*
 * Copyright 2016 husky Team
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

#include "base/log.hpp"
#include "core/engine.hpp"

#include "densevector.hpp"
#include "lshfactory.hpp"
#include "lshutils.hpp"

using std::vector;
namespace husky {
namespace losha {

template<typename ItemIdType,
    typename ItemElementType,
    typename QueryMsg = DenseVector<ItemIdType, ItemElementType>,
    typename AnswerMsg = std::pair<ItemIdType, float> >
class LSHQuery: public DenseVector<ItemIdType, ItemElementType> {
    public:

        // to store buckets, for each bucket, we will send the query
        static thread_local std::vector<  std::vector<int> > query_msg_buffer;
        bool needBroadcast = false;

        QueryMsg queryMsg;

        // require by husky object
        explicit LSHQuery(const typename LSHQuery::KeyT& id) : DenseVector<ItemIdType, ItemElementType>(id) {};

        const DenseVector<ItemIdType, ItemElementType>& getQuery() {
            return this->getItem();
        }

        // bId must contain table Idx as the last element
        inline void sendToBucket(const std::vector<int>& bId) {
            query_msg_buffer.push_back(bId);
        }

        // sig cannot contain table Idx
        inline void sendToBucket(std::vector<int> sig, int tableIdx) {
            sig.push_back(tableIdx);
            query_msg_buffer.push_back(sig);
        }

        virtual void query(
            LSHFactory<ItemIdType, ItemElementType>& factory,
            const vector<AnswerMsg>& inMsg) = 0;

        void broadcast() {
            needBroadcast = true;
        }
};

template<typename ItemIdType, typename ItemElementType,
    typename QueryMsg, typename AnswerMsg>
thread_local std::vector<std::vector<int>>
    LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>::query_msg_buffer;

} // namespace losha
} // namespace husky
