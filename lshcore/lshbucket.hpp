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
#include <string>
#include <utility>
#include <vector>

#include "densevector.hpp"
#include "lshfactory.hpp"
#include "lshutils.hpp"

namespace husky {
namespace losha { 

template<typename ItemIdType, typename ItemElementType,
    typename QueryMsg,
    typename AnswerMsg = std::pair<ItemIdType, float> >
class LSHBucket {
    public:
        using KeyT = std::vector<int>;
        KeyT bucketId_;
        std::vector<ItemIdType> itemIds_;

        explicit LSHBucket(const typename LSHBucket::KeyT& bId): bucketId_(bId) {}
        const KeyT& id() const { return bucketId_;}

//       this function may incur std::bad_cast problem;
        // inline std::vector<QueryMsg>& getQueries(
        //         LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg >& bk) {
        //     return Husky::get_messages<QueryMsg>(bk);
        // }

        // explicit LSHBucket(const typename LSHBucket::KeyT& id): bucketId_(id) {}
        virtual void forward(LSHFactory<ItemIdType, ItemElementType>& factory) {
        }

        // std::string toString() {
        //     std::string str = "(bucketId_: " + std::to_string(bucketId_) + " -> ";
        //     str += std::to_string(this->itemIds_);
        //     // for (auto& i : this->itemIds_) {
        //     //     str += i.toString() + ", ";
        //     // }
        //     str += ")";
        //     return str;
        // }

};

} // namespace losha
} // namespace husky
