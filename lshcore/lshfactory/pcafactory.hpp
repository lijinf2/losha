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

#include <map>
#include <string>
#include <vector>

#include "lshcore/densevector.hpp"
#include "lshcore/lshfactory.hpp"
#include "lshcore/lshutils.hpp"

#include "lshcore/lshfactory/pcahasher.hpp"
#include "losha/common/distor.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class PCAFactory:
    public LSHFactory<ItemIdType, ItemElementType> {
private:
    PCAHasher<ItemElementType> hasher;
public:

    void initialize(const string& modelFile) {
        hasher.loadModel(modelFile);
        this->_band = hasher.getBand();
        this->_row = hasher.getRow();
        this->_dimension = hasher.getDimension();
    }

    // return signatures of each band
    // in the format of std::vector< std::vector<int> >
    std::vector< std::vector<int> > calSigs (
        // const DenseVector<ItemIdType, ItemElementType> &p) override {
        const vector<ItemElementType> &itemVector) override {

        std::vector< std::vector<int> > signatureInBands;
        signatureInBands.reserve(this->getBand());

        for (int i = 0; i < this->_band; ++i) {
            signatureInBands.emplace_back(hasher.getBuckets(i, itemVector.data()));
        }
        return signatureInBands;
    }

    // return projections of each band
    // in the format of std::vector< std::vector<float> >
    virtual std::vector< std::vector<float> > calProjs(
        const DenseVector<ItemIdType, ItemElementType> &p) {
        const auto& itemVector = p.getItemVector();

        std::vector< std::vector<float> > projectionsInBands;
        projectionsInBands.reserve(this->getBand());

        for (int i = 0; i < this->_band; ++i) {
            projectionsInBands.emplace_back(hasher.getHashFloats(i, itemVector.data()));
        }
        return projectionsInBands;
    }

    virtual float calDist(
            const std::vector<ItemElementType> & queryVector,
            const std::vector<ItemElementType> & itemVector) override {
        return calE2Dist(queryVector, itemVector);
    }

};

} // namespace losha
} // namespace husky
