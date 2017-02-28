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

// support both denseVector and sparseVector by simulate sparseVector by denseVector

#pragma once
#include <math.h>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "densevector.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class CosLSHFunction {
public:
    CosLSHFunction() {}
    explicit CosLSHFunction(std::vector<float>& a) {
        _a.swap(a);
        if(_a.capacity() != _a.size())
            _a.shrink_to_fit();
    }

    // for sparseVector
    inline float dotProduct(
            const std::vector< std::pair<int, ItemElementType> >& v2) {
        float product = 0;
        for (int i = 0; i < v2.size(); ++i) {
            product += _a[ v2[i].first ] * v2[i].second;
        }
        return product;
    }

    // for denseVector
    inline float dotProduct(const std::vector<ItemElementType>& v2) {
        assert(_a.size() >= v2.size());
        float product = 0;
        for (int i = 0; i < _a.size(); ++i) {
            product += _a[i] * v2[i];
        }
        return product;
    }

    // for sparseVector
    inline float getProjection(
            const std::vector< std::pair<int, ItemElementType> >& itemVector) {
        return this->dotProduct(itemVector);
    }

    // for denseVector
    inline float getProjection(const std::vector<ItemElementType>& itemVector) {
        return this->dotProduct(itemVector);
    }

    // for sparseVector
    inline bool getQuantization(
            const std::vector< std::pair<int, ItemElementType> >& itemVector) {
        if (this->getProjection(itemVector) >= 0)
            return true;
        else
            return false;
    }

    // for denseVector
    bool getQuantization(std::vector<ItemElementType>& itemVector) {
        if (this->getProjection(itemVector) >= 0)
            return true;
        else
            return false;
    }

    inline int getDimension(){
        return _a.size();
    }

    // for visualization
    std::string toString() {
        std::string str = "(";
        for (auto& e : _a) {
            str += std::to_string(e) + " ";
        }
        str += ")";
        return str;
    }

    // wrapper for DenseVector
    inline float getProjection(
            const DenseVector<ItemIdType, std::pair<int, ItemElementType> >& p) {
        const std::vector<std::pair<int, ItemElementType> >& itemVector = p.getItemVector();
        return this->dotProduct(itemVector);
    }

    // wrapper for DenseVector
    inline float getProjection(const DenseVector<ItemIdType, ItemElementType>& p) {
        const std::vector<ItemElementType>& itemVector = p.getItemVector();
        return this->dotProduct(itemVector);
    }

    // wrapper for DenseVector 
    bool getQuantization(
            const DenseVector<ItemIdType, std::pair<int, ItemElementType> >& p) {
        if (this->getProjection(p) >= 0)
            return true;
        else
            return false;
    }

    // wrapper for DenseVector
    bool getQuantization(const DenseVector<ItemIdType, ItemElementType>& p) {
        if (this->getProjection(p) >= 0)
            return true;
        else
            return false;
    }


private:
    std::vector<float> _a;
};

} // namespace losha
} // namespace husky
