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

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class L2HFunction {
    public:
        L2HFunction() {}
        L2HFunction(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b) {
            this->pca.swap(a);
            this->rotation.swap(b);
        }

        void initialize(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b) {
            this->transformation.swap(a);
            this->rotation.swap(b);
        }

        template<typename T>
        inline float dotProduct(const std::vector<float> &a, const std::vector<T>& v2) {
            assert(a.size() == v2.size());
            float product = 0;
            for (int i = 0; i < a.size(); ++i) {
                product += a[i] * v2[i];
            }
            return product;
        }

        inline std::vector<float> getProjection(
                const std::vector<ItemElementType>& itemVector) {
            std::vector<float> pca; 
            for (int c = 0; c < transformation.size(); ++c) {
                pca.push_back( dotProduct(transformation[c], itemVector));
            }
            std::vector<float> projection;
            for (int r = 0; r < rotation.size(); ++r) {
                projection.push_back(dotProduct(rotation[r], pca));
            }
            return projection;
        }

        inline std::vector<bool> getQuantization(
                const std::vector<ItemElementType>& itemVector) {
            std::vector<float> projection= getProjection(itemVector);
            std::vector<bool> bits;
            for (int i = 0; i < projection.size(); ++i) {
                if (projection[i] > 0) {
                    bits.push_back(1);
                } else {
                    bits.push_back(0);
                }
            }
            return bits;
        }

        std::string toString(){
            // output pca matrix
            std::string log = "transformation X D, i.e. ";
            log += std::to_string(transformation.size()) + " X ";
            log += std::to_string(transformation[0].size()) + "\n";
            for (int i = 0; i < transformation.size(); ++i) {
                for (int j = 0; j < transformation[i].size(); ++j) {
                    log += std::to_string(transformation[i][j]) + " ";
                }
                log += "\n";
            }

            // output rotation matrix
            std::log = "rotation d X d, i.e. ";
            log += std::to_string(rotation.size()) + " X ";
            log += std::to_string(rotation[0].size()) + "\n";
            for (int i = 0; i < rotation.size(); ++i) {
                for (int j = 0; j < rotation[i].size(); ++j) {
                    log += std::to_string(rotation[i][j]) + " ";
                }
                log += "\n";
            }
            return log;
        }
        // the below are wrappers
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

    private:
        // transformation.size() = d, and each row has D element, for friendly cache
        std::vector<std::vector<float>> transformation; // a D * d matrix, D is the dimensionality of the data and d is projected dimensionality 
        std::vector<std::vector<float>> rotation;  // a d * d matrix
};

} // namespace losha
} // namespace husky
