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
#include <cmath>
#include <string>
#include <vector>

// #include "boost/tokenizer.hpp"
// #include "base/log.hpp"
// #include "core/engine.hpp"
#include "core/engine.hpp"

#include "lshcore/lshutils.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class DenseVector {
public:
    using KeyT = ItemIdType;
    ItemIdType _itemId;
    std::vector<ItemElementType> _itemVector;

    DenseVector() {}

    // require by Husky object
    const KeyT& id() const { return _itemId;}
    // require by Husky object
    explicit DenseVector(const KeyT& id) { _itemId = id;}

    DenseVector(ItemIdType& id, std::vector<ItemElementType>& v) {
        _itemId = id;
        _itemVector.swap(v);
        if (_itemVector.capacity() != _itemVector.size()) {
            _itemVector.shrink_to_fit();
        }
    }

    void setItemId(ItemIdType& id) {
        _itemId = id;
    }

    void setItemVector(std::vector<ItemElementType>& itemVector) {
        _itemVector.swap(itemVector);
        if (_itemVector.capacity() != _itemVector.size()) {
            _itemVector.shrink_to_fit();
        }
    }

    const std::vector<ItemElementType>& getItemVector() const {
        return _itemVector;
    }

    int getItemVectorSize() {
        return _itemVector.size();
    }

    const ItemIdType getItemId() const {
        return _itemId;
    }

    const DenseVector<ItemIdType, ItemElementType>& getItem() const {
        return *this;
    }

    friend husky::BinStream & operator<<(husky::BinStream & stream, DenseVector<ItemIdType, ItemElementType>& p) {
        stream << p.itemId << p.itemVector;
        return stream;
    }

    friend husky::BinStream & operator>>(husky::BinStream & stream, DenseVector<ItemIdType, ItemElementType>& p) {
        stream >> p.itemId >> p.itemVector;
        return stream;
    }

    friend std::string to_string(const DenseVector<ItemIdType, ItemElementType>& p) {
        std::string result = "(id: " + std::to_string(p.getItemId()) + "\titemVector: "
            + std::to_string(p.getItemVector()) + ")\n";
        return result;
    }

    std::string toString(){
        std::string str = "";
        str += std::to_string(getItemId());
        for (const auto& e : getItemVector()) {
            str += " ";
            str += std::to_string(e);
        }
        return str;
    }
};

} // namespace losha
} // namespace husky
