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
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include "base/log.hpp"
#include "gqr/include/base/basehasher.h"
using namespace lshbox;
using std::vector;
using std::string;
using std::ifstream;
using std::istringstream;

namespace husky {
namespace losha {

template<typename DATATYPE>
class PCAHasher:
    public BaseHasher<DATATYPE, vector<int>> {

private:
    vector<vector<vector<float> > > pcsAll;
    vector<float> mean;

public:

    PCAHasher() : BaseHasher<DATATYPE, vector<int>>()  {}

    void loadModel(const string& modelFile, const string& uselessParameter = "") override; 

    virtual vector<float> getHashFloats(unsigned k, const DATATYPE *domin) const ;

    vector<int> getBuckets(unsigned k, const DATATYPE *domin) const ;

    unsigned getBand() const {
        return pcsAll.size();
    }

    unsigned getRow() const {
        return pcsAll[0].size();
    }

    unsigned getDimension() const {
        return mean.size();
    }
};

//--------------------- Implementations ------------------
template<typename DATATYPE>
void PCAHasher<DATATYPE>::loadModel(const string& modelFile, const string& uselessParameter) {
    string line;
    // initialized statistics and model
    ifstream modelFin(modelFile.c_str());
    if (!modelFin) {
        husky::LOG_I << "cannot open file " << modelFile << std::endl;
        assert(false);
    }
    getline(modelFin, line);
    istringstream statIss(line);
    int modelNumTable, modelNumFeature, modelCodelen, modelNumItem, modelNumQuery;
    statIss >> modelNumTable >> modelNumFeature >> modelCodelen >> modelNumItem >> modelNumQuery;

    // mean 
    this->loadFloatVector(modelFin, modelNumFeature).swap(mean);


    // hash functions
    this->pcsAll.resize(modelNumTable);
    for (int tb = 0; tb < modelNumTable; ++tb) {
        this->loadFloatMatrixTranspose(modelFin, modelNumFeature, modelCodelen).swap(pcsAll[tb]);
    }
}


template<typename DATATYPE>
vector<float> PCAHasher<DATATYPE>::getHashFloats(unsigned tableIdx, const DATATYPE *data) const
{
    // project
    return this->getProjection(data, pcsAll[tableIdx], mean);
}

template<typename DATATYPE>
vector<int> PCAHasher<DATATYPE>::getBuckets(unsigned tableIdx, const DATATYPE *data) const
{
    vector<float> hashFloats = getHashFloats(tableIdx, data);
    if (hashFloats.size() > 32) {
        husky::LOG_I << "pcahasher supports maximum 32 bits, but this app uses " << hashFloats.size() << "bits" << std::endl;
        assert(false);
    }

    int value = 0;
    for (int i = 0; i < hashFloats.size(); ++i) {
        value <<= 1;
        if (hashFloats[i] >=0) 
            value += 1;
    }
    vector<int> bucket;
    bucket.push_back(value);
    return bucket;
}
} // namespace losha
} // namespace husky
