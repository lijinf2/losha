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

#include "core/engine.hpp"

#include "densevector.hpp"
#include "e2lshfunction.hpp"
#include "lshfactory.hpp"
#include "lshutils.hpp"

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class E2LSHFactory:
    public LSHFactory<ItemIdType, ItemElementType> {
public:
    // hashFunctions on each worker must be the same
    std::vector<E2LSHFunction<ItemIdType, ItemElementType>> hashFunctions;    float W;  // segment

    // void initialize(std::map<std::string, std::string>& params) {
    //     assert(params.find("bands") != params.end());
    //     assert(params.find("rows") != params.end());
    //     assert(params.find("W") != params.end());
    //     assert(params.find("dimension") != params.end());
    //
    //     int bands = stoi(params["bands"]);
    //     int rows = stoi(params["rows"]);
    //     float W = stof(params["W"]);
    //     int dimension = stoi(params["dimension"]);
    //     int seed = 0;
    //     if (params.find("seed") != params.end()) seed = stoi(params["seed"]);
    //
    //     this->initialize(bands, rows, W, dimension, seed);
    // }

    void loadGQRModel() {
    }

    void initialize(int bands, int rows, int dimension, float w, int seed = 0) {
        this->_band = bands;
        this->_row = rows;
        this->W = w;
        this->_dimension = dimension;
        generateE2LSHFunctions(seed);
    }

    // generate hash functions
    void generateE2LSHFunctions(int seed) {
        //
        // std::default_random_engine generator(seed);
        std::default_random_engine generator(10042);
        std::normal_distribution<float> distribution(0.0, 1.0);

        int numFunctions = this->_band * this->_row;

        std::vector<float> a;
        a.resize(this->_dimension);
        float b;

        std::uniform_real_distribution<float> uniRealDst(0, this->W);
        for (int i = 0; i < numFunctions; ++i) {
        // attention!!! should set seed for each hash Function!!!
        // otherwise the performance can be very bad, refer LayedLSH
        // the setting is the same as layed LSH
         generator.seed(i);

            for (int j = 0; j < this->_dimension; ++j) {
                a[j] = distribution(generator);
            }

            // thread insafe
            // b = static_cast<float>(rand()) /
            //     static_cast<float> (rand_max / this->w);

            b = uniRealDst(generator);

            // Husky::log_msg("generate b " + std::to_string(b) + "\tby worker" + std::to_string(get_worker().id));
            E2LSHFunction<ItemIdType, ItemElementType> func(a, b, this->W);
            hashFunctions.push_back(func);
        }
    }

    // report hash functions generated
    void reportE2LSHFunctions() {
        std::string parametersLog = "worker"
            + std::to_string(husky::Context::get_global_tid()) + ": ";

        parametersLog += "rows: " + std::to_string(this->_row) +
                         ", bands: " + std::to_string(this->_band) +
                         ", dimensions: " + std::to_string(this->_dimension) +
                         ", W: " + std::to_string(W);


        std::string hashFunctionsLog = "hash functions:";
        husky::LOG_I << hashFunctionsLog << std::endl;;
        for (auto &h : hashFunctions) {
            parametersLog += "\n" +  h.toString();
        }
        husky::LOG_I << parametersLog << std::endl;
    }

    // return all signatures, as vector<int>
    // in the format of std::vector< int >
    std::vector<int> calSignatures(
        const vector<ItemElementType>& p) {
        std::vector<int> allSignatures;
        allSignatures.resize(this->hashFunctions.size());

        int index = 0;
        for (auto& fun : this->hashFunctions) {
            allSignatures[index++] = fun.getBucket(p);
        }
        return allSignatures;
    }

    // return signatures of each band
    // in the format of std::vector< std::vector<int> >
    std::vector< std::vector<int> > calSigs (
        // const DenseVector<ItemIdType, ItemElementType> &p) override {
        const vector<ItemElementType> &itemVector) override {
        std::vector<int> allSignatures = this->calSignatures(itemVector);

        std::vector< std::vector<int> > signatureInBands;

        std::vector<int> vec;
        vec.resize(this->_row);
        for (int i = 0; i < this->_band; ++i) {
            int start = i * this->_row;
            for (int j = 0; j < this->_row; ++j) {
                vec[j] = allSignatures[start + j];
            }
            signatureInBands.push_back(vec);
        }
        return signatureInBands;
    }

    // return all projections
    // in the format of std::vector< float >
    std::vector<float> calProjections(
        const DenseVector<ItemIdType, ItemElementType>& p) {
        std::vector<float> allProjections;
        allProjections.resize(this->hashFunctions.size());

        int index = 0;
        for (auto& fun : this->hashFunctions) {
            allProjections[index++] = fun.getProjection(p);
        }
        return allProjections;
    }

    // return projections of each band
    // in the format of std::vector< std::vector<float> >
    virtual std::vector< std::vector<float> > calProjs(
        const DenseVector<ItemIdType, ItemElementType> &p) {
        std::vector<float> allProjections = this->calProjections(p);

        std::vector< std::vector<float> > projectionsInBands;

        std::vector<float> vec;
        vec.resize(this->_row);
        for (int i = 0; i < this->_band; ++i) {
            int start = i * this->_row;
            for (int j = 0; j < this->_row; ++j) {
                vec[j] = allProjections[start + j];
            }
            projectionsInBands.push_back(vec);
        }
        return projectionsInBands;
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

    // for sparse vector
    // virtual float calDist(
    //        const DenseVector<ItemIdType, std::pair<int, ItemElementType> > & query,
    //        const DenseVector<ItemIdType, std::pair<int, ItemElementType> > & item) override {
    //     const std::vector<std::pair<int, ItemElementType> > &queryVector = query.getItemVector();
    //     const std::vector<std::pair<int, ItemElementType> > &itemVector = item.getItemVector();
    //     assert(queryVector.size() == itemVector.size());
    //
    //     typename std::vector<std::pair<int, ItemElementType> >::const_iterator qIt = queryVector.begin();
    //     typename std::vector<std::pair<int, ItemElementType> >::const_iterator itemIt = itemVector.begin();
    //
    //     float distance = 0;
    //     while(itemIt != itemVector.end() && qIt != queryVector.end()) {
    //         if ((*itemIt).first < (*qIt).first) {
    //             itemIt++;
    //         }else if ((*itemIt).first > (*qIt).first) {
    //             qIt++;
    //         }else {
    //             distance += ((*itemIt).second - (*qIt).second) * ((*itemIt).second - (*qIt).second);
    //             ++itemIt; ++qIt;
    //         }
    //     }
    //     return sqrt(distance);
    // }
};

} // namespace losha
} // namespace husky
