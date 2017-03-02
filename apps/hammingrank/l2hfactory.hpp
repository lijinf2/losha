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

#include <fstream>
#include <string>
#include <vector>

#include "core/engine.hpp"

#include "lshcore/densevector.hpp"
#include "lshcore/lshfactory.hpp"
#include "l2hfunction.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class L2HFactory:
    public LSHFactory<ItemIdType, ItemElementType> {
public:
    L2HFunction<ItemIdType, ItemElementType> hashFunctions; 

    // load from disk and initialize
    void initialize(int bands, int rows, int dimension, const std::string& filePath, int BIDBYTE) {
        this->_band = 1;
        this->_row = rows;
        this->_dimension = dimension;

        // read parameters from disk dumped by LSHBOX
        std::ifstream fin(filePath, std::ios::binary);
        unsigned paramM, paramL, paramD, paramN, paramS;
        fin.read((char*)&paramM, sizeof(unsigned));
        fin.read((char*)&paramL, sizeof(unsigned));
        fin.read((char*)&paramD, sizeof(unsigned));
        fin.read((char*)&paramN, sizeof(unsigned));
        fin.read((char*)&paramS, sizeof(unsigned));
        assert(paramL == 1);
        assert(paramD == this->_dimension);
        assert(paramN == this->_row);

        // ignore variable rndArray
        fin.ignore(sizeof(unsigned) * paramN);

        // ignore variable tables
        unsigned count;
        fin.read((char*) &count, sizeof(unsigned));
        for (unsigned i = 0; i != count; ++i) {
            fin.ignore(BIDBYTE);
            unsigned length;
            fin.read((char*) &length, sizeof(unsigned));
            fin.ignore(sizeof(unsigned) * length);
        }

        // read pca matrix and rotation matrix
        std::vector<std::vector<float>> transformation;
        std::vector<std::vector<float>> rotation;
        transformation.resize(this->_row);  // d row, and D element each row
        rotation.resize(this->_row);  // d row, and d element each row
        for (int i = 0; i < this->_row; ++i) {
            transformation[i].resize(this->_dimension);
            fin.read((char*)&transformation[i][0], sizeof(float) * this->_dimension);
            rotation[i].resize(this->_row);
            fin.read((char*)&rotation[i][0], sizeof(float) * this->_row);
        }
        hashFunctions.initialize(transformation, rotation);
        fin.close();
    }

    // report hash functions generated
    std::string toString() {

        std::string log = "";
        log += "row: " + std::to_string(this->_row)
            + ", bands " + std::to_string(this->_band)
            + ", dimension: " + std::to_string(this->_dimension);


        log += ", and functions:";
        log += "\n";
        log += hashFunctions.toString();
        return log;
    }

    std::vector< std::vector<int> > calSigs (
        const vector<ItemElementType> &itemVector) override {

        std::vector<bool> bits = hashFunctions.getQuantization(itemVector);
        assert(bits.size() == this->_row); // only use one hash table

        // transform bits to vector<int>
        std::vector<int> allSignatures;
        int bitSetIndex = 0;
        int intValue = 0;
        for (int i = 0; i < bits.size(); ++i) {
            if (bitSetIndex < 32) {
                intValue <<= 1;
                intValue += 
                bitSetIndex++;
            } else {
                allSignatures.push_back(intValue);
                bitSetIndex = 0;
            }
        }
        if (bitSetIndex != 0)
            allSignatures.push_back(intValue);

        std::vector< std::vector<int> > signatureInBands;
        signatureInBands.push_back(allSignatures);
        return signatureInBands;
    }

    std::vector< std::vector<float> > calProjs(
        const std::vector<ItemElementType> &itemVector) override {

        assert(this->_band == 1);
        std::vector<std::vector<float>> allProjections;
        allProjections.push_back(hashFunctions.getProjection(itemVector));
        return allProjections;
    }

    virtual float calDist(
            const std::vector<ItemElementType> & queryVector,
            const std::vector<ItemElementType> & itemVector) override {
        typename std::vector<ItemElementType>::const_iterator qIt = queryVector.begin();

        typename std::vector<ItemElementType>::const_iterator myIt = itemVector.begin();

        float distance = 0;
        while (myIt != itemVector.end() && qIt != queryVector.end()) {
            distance += (*myIt - *qIt) * (*myIt - *qIt);
            ++myIt; ++qIt;
        }
        return sqrt(distance);
    }
};

} // namespace losha
} // namespace husky
