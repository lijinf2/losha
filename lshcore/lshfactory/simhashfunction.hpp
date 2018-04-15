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

#include "losha/common/dotproduct.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class SimHashFunction {
public:
    SimHashFunction() {}
    explicit SimHashFunction(std::vector<float>& a) {
        _a.swap(a);
        if(_a.capacity() != _a.size())
            _a.shrink_to_fit();
    }

    // for denseVector
    inline float getProjection(
        const std::vector<ItemElementType>& itemVector) const {
        return dotProduct(_a, itemVector);
    }

    // for denseVector
    bool getBucket(
        const std::vector<ItemElementType>& itemVector) const {
        if (this->getProjection(itemVector) >= 0)
            return true;
        else
            return false;
    }

private:
    std::vector<float> _a;
};

} // namespace losha
} // namespace husky
