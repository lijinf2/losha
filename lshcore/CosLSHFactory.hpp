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

#include "base/log.hpp"

#include "CosLSHFunction.hpp"
#include "DenseVector.hpp"
#include "LSHAlgebra.hpp"
#include "LSHFactory.hpp"
#include "LSHUtils.hpp"

#define PI 3.141592654
template<typename ItemIdType, typename ItemElementType>
class CosLSHFactory:
    public LSHFactory<ItemIdType, ItemElementType> {
public:
    std::vector< CosLSHFunction<ItemIdType, ItemElementType> >  hashFunctions;

    /* Set parameters
     * */
    void initialize() {
        initialize(Husky::Context::get_params());
    }

    void initialize(std::map<std::string, std::string>& params) {
        assert(params.find("bands") != params.end());
        assert(params.find("rows") != params.end());
        assert(params.find("dimension") != params.end());


        int bands = stoi(params["bands"]);
        int rows = stoi(params["rows"]);
        int dimension = stoi(params["dimension"]);
        int seed = 0;
        if (params.find("seed") != params.end()) seed = stoi(params["seed"]);

        this->initialize(bands, rows, dimension, seed);
    }

    void initialize(int bands, int rows, int dimension, int seed) {
        this->bands = bands;
        this->rows = rows;
        this->dimension = dimension;

        generateCosLSHFunctions(seed);
    }

    // generate hash functions
    void generateCosLSHFunctions(int seed) {
        std::default_random_engine generator(seed);
        std::normal_distribution<float> distribution(0.0, 1.0);

        int numFunctions = this->bands * this->rows;

        std::vector<float> a;
        a.resize(this->dimension);

        for (int i = 0; i < numFunctions; ++i) {
            for (int j = 0; j < this->dimension; ++j) {
                a[j] = distribution(generator);
            }

            LSHContext::normalize(a);
            CosLSHFunction<ItemIdType, ItemElementType> func(a);
            hashFunctions.push_back(func);
        }

        // if (get_worker().id == 0) {
        //     this->reportE2LSHFunctions();
        // }
    }

    // the below is for sparseVector
    // for denseVector
    std::vector<bool> calSignaturesInBool(
        const DenseVector<ItemIdType, ItemElementType>& p) {
        std::vector<bool> allSignatures;
        allSignatures.resize(this->hashFunctions.size());

        int index = 0;
        for (auto& fun : this->hashFunctions) {
            allSignatures[index++] = fun.getBucket(p);
        }
        return allSignatures;
    }

    // compress into std vector<int>
    virtual std::vector< std::vector<int> > calSignatureInBands(
        const DenseVector<ItemIdType, ItemElementType> &p) {
        int numRows = LSHContext::getRows();
        int numBands = LSHContext::getBands();

        std::vector<bool> allSignatures = this->calSignaturesInBool(p);

        std::vector< std::vector<int> > signatureInBands;

        std::vector<int> vec;
        vec.resize(std::ceil(numRows / 32.0));
        // std::bitset< 32 > oneInt; //default is 0

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
                    // oneInt.set(bitSetIndex++, allSignatures[start + j]);
                    intValue = (intValue << 1) + allSignatures[start + j];
                    bitSetIndex++;
                } else {
                    // vec[vecIndex++] = static_cast<int>(oneInt.to_ullong());
                    vec[vecIndex++] = intValue;
                    bitSetIndex = 0;
                }
            }
            if (bitSetIndex != 0)
                    vec[vecIndex++] = intValue;

            signatureInBands.push_back(vec);
        }
        return signatureInBands;
    }

    // for denseVector, NO ASSUMPTION a * b / |a| / |b|
    virtual float calDistance(
           const std::vector<ItemElementType> & queryVector,
           const std::vector<ItemElementType> & itemVector) {
        typename std::vector<ItemElementType>::const_iterator qIt = queryVector.begin();
        typename std::vector<ItemElementType>::const_iterator myIt = itemVector.begin();

        float cosTheta = 0;
        while (myIt != itemVector.end() && qIt != queryVector.end()) {
            cosTheta += (*myIt) * (*qIt);
            ++myIt; ++qIt;
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
        // //assume no zero-vector
        // similarity /= getL2Norm(queryVector);
        // similarity /= getL2Norm(itemVector);
        // return 1 - similarity;
    }

    // report hash functions generated
    virtual std::string LSHFunctionsToString()  {
        std::string parametersLog = "\n\nworker" +
                                    std::to_string(get_worker().id) + ": ";

        parametersLog += "rows: " + std::to_string(this->rows) +
                         ", bands: " + std::to_string(this->bands) +
                         ", dimensions: " + std::to_string(this->dimension);

        int index = 0;
        std::string hashFunctionStr = "hash function";
        // get only the first function
        for (int i = 0; i < this->getRows(); ++i)
        parametersLog += "\n" + hashFunctionStr + std::to_string(i) +
            ": " +hashFunctions[i].toString();
        parametersLog += "\n\n";
        return parametersLog;
    }
};
