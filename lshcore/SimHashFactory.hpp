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

// Assume indices in sparse vector sorted in increasing order

#pragma once
#include <map>
#include <string>
#include <vector>

#include "coslshfunction.hpp"
#include "densevector.hpp"
#include "LSHAlgebra.hpp"
#include "lshfactory.hpp"
#include "lshutils.hpp"

using std::vector;
using std::pair;

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class SimHashFactory:
    public LSHFactory < ItemIdType, pair<int, ItemElementType> > {

public:
    
    int _seed;
    vector< CosLSHFunction<ItemIdType, ItemElementType> >  _hashFunctions;

    void initialize(int band, int row, int dimension, int seed=0) {
        this->_band = band;
        this->_row = row;
        this->_dimension = dimension;
        _seed = seed;
        
        generateCosLSHFunctions();
    }

    // generate hash functions
    void generateCosLSHFunctions() {

        std::default_random_engine generator(_seed);
        std::normal_distribution<float> distribution(0.0, 1.0);

        int numFunctions = this->_band * this->_row;
        _hashFunctions.resize(numFunctions);

        for (int i = 0; i < numFunctions; ++i) {
            
            vector<float> a;
            a.resize(this->_dimension);
            for (int j = 0; j < this->_dimension; ++j) {
                a[j] = distribution(generator);
            }
            normalize(a);

            CosLSHFunction<ItemIdType, ItemElementType> func(a);
            _hashFunctions[i] = func;
            //_hashFunctions.push_back(func);
        }
        
    }

    // INPUT: SparseVector
    // Calculate dot product between hash function and item vector
    // Then do quantization
    virtual vector<bool> calSignaturesInBool(
        const vector< pair<int, ItemElementType> >& itemVector) {
        
        vector<bool> allSignatures;
        allSignatures.resize(_hashFunctions.size());

        int index = 0;
        for (auto& fun : _hashFunctions) {
            allSignatures[index++] = fun.getQuantization(itemVector);
        }
        return allSignatures;
    }

    // Compression
    // convert vector<bool> to vector<int>
    virtual vector< vector<int> > calSigs(
        const vector< pair<int, ItemElementType> > &itemVector) override {
        
        // Number of bits for each hash table
        // Determine the number of buckets in a hash table
        int numRows = this->_row;
        // Determine the number of hash table we have
        int numBands = this->_band;
        // Number of int in each signature (after compression)
        int intNum = std::ceil(numRows / 32.0);

        vector<bool> allSignatures = this->calSignaturesInBool(itemVector);
        vector< vector<int> > signatureInBands;
        signatureInBands.resize( numBands );
        for (int i = 0; i < numBands; ++i) 
        {
            for (int j = 0; j < intNum; ++j)
            {
                signatureInBands[i].push_back(0);
            }
        }

        int vecIndex;
        int bitSetIndex;
        int intValue;

        for (int i = 0; i < numBands; ++i) {
            int start = i * numRows;
            vecIndex = 0;
            bitSetIndex = 0;
            intValue = 0;

            // compress bool vector into int vector
            // may have problem since his is not unsigned int
            for (int j = 0; j < numRows; ++j) {
                if (bitSetIndex < 32) {
                    intValue = (intValue << 1) + allSignatures[start + j];
                    bitSetIndex++;
                } else {
                    signatureInBands[i][vecIndex++] = intValue;
                    intValue = 0;
                    intValue = (intValue << 1) + allSignatures[start + j];
                    bitSetIndex = 1;
                }
            }
            if (bitSetIndex != 0){
                signatureInBands[i][vecIndex++] = intValue;
            }

        }
        return signatureInBands;
    }

    virtual float calDist(
           const vector< pair<int, ItemElementType> > & queryVector,
           const vector< pair<int, ItemElementType> >& itemVector) override{
        
        typename vector< pair<int, ItemElementType> >::const_iterator qIt = queryVector.begin();
        typename vector< pair<int, ItemElementType> >::const_iterator itemIt = itemVector.begin();

        float cosTheta = 0;
        while (itemIt != itemVector.end() && qIt != queryVector.end()) {
            if( (*itemIt).first < (*qIt).first )
            {
                itemIt++;
            }
            else if( (*itemIt).first > (*qIt).first )
            {
                qIt++;
            }
            else
            {
                cosTheta += ( (*itemIt).second ) * ( (*qIt).second );
                ++itemIt; ++qIt;
            }
        }

        if (cosTheta > 1) {
            assert(cosTheta - 1 < 0.001);
            cosTheta = 1.0;
        } else if (cosTheta < -1) {
            assert(-cosTheta - 1 < 0.001);
            cosTheta = -1.0;
        }
        // return angle in the interval [0, pi] radians
        return acos(cosTheta);
    }

};

} // namespace losha
} // namespace husky
