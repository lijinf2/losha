/*
 * Copyright 2016 Husky Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <math.h>
#include <random>
#include <string>
#include <vector>

#include "lshcore/densevector.hpp"
#include "losha/common/dotproduct.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class E2LSHFunction {
    public:
        E2LSHFunction() {}
        E2LSHFunction(std::vector<float> a, float b, float W) {
            this->a.swap(a);
            this->b = b;
            this->W = W;
        }

        inline float getProjection(
                const std::vector<ItemElementType>& itemVector) const {
            float product = dotProduct(this->a, itemVector);
            return product + this->b;
        }

        int getQuantization(
                const std::vector<ItemElementType>& itemVector) const {
            return static_cast<int>(floor(this->getProjection(itemVector) / this->W));
        }

        int getBucket(
                const std::vector<ItemElementType>& itemVector) const {
            return this->getQuantization(itemVector);
        }
        
        // return hash value
        // int getBucket(const DenseVector<ItemIdType, ItemElementType>& p) {
        //     const std::vector<ItemElementType>& itemVector = p.getItemVector();
        //     return static_cast<int>(floor(this->getProjection(p) / this->W));
        // }

        inline float getProjection(
                const DenseVector<ItemIdType, ItemElementType>& p) {
            const std::vector<ItemElementType>& itemVector = p.getItemVector();
            return getProjection(itemVector);
        }

        int getQuantization(
                const DenseVector<ItemIdType, ItemElementType>& p) {
            const std::vector<ItemElementType>& itemVector = p.getItemVector();
            return getQuantization(itemVector);
        }

        std::string toString() {
            std::string str = "h: (";
            for (auto& e : a) {
                str += std::to_string(e) + " ";
            }
            str += ", " + std::to_string(this->b) + ", "
                + std::to_string(this->W) + ")";
            return str;
        }

    private:
        std::vector<float> a;
        float b;
        float W;
};

} // namespace losha
} // namespace husky
