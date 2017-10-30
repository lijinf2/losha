/*
 * Copyright 2016 husky Team
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

// Assume indices in sparse vector sorted in increasing order

#pragma once
#include <vector>

#include "lshcore/coslshfunction.hpp"
#include "lshcore/densevector.hpp"
#include "lshcore/lshfactory.hpp"
#include "lshcore/lshutils.hpp"

using std::vector;
using std::pair;

namespace husky {
namespace losha {
template<typename ItemIdType, typename ItemElementType>
class SparseCosLSHFactory:
    public LSHFactory<ItemIdType, pair<int, ItemElementType> > { 
public:
    int _seed;
    vector< CosLSHFunction<ItemIdType, ItemElementType> >  _hashFunctions;

    SparseCosLSHFactory() {}
    // initilization with band, row, dimensions and seed
    void initialize(int band, int row, int dimension, int seed = 0) {
        this->_band = band;
        this->_row = row;
        this->_dimension = dimension;
        _seed = seed;

        generateFunctions();
    }

    void generateFunctions(){
        int numFunctions = this->_band * this->_row;
        _hashFunctions.resize(numFunctions);

        std::default_random_engine generator(_seed);
        std::normal_distribution<float> distribution(0.0, 1.0);

        for (int i = 0; i < numFunctions; ++i) {
            // The performance will be bad if we set seed in each iteration, which is interestingly opposite to E2LSH
            // generator.seed(i);

            vector<float> a;
            a.resize(this->_dimension);
            for (int j = 0; j < this->_dimension; ++j) {
                a[j] = distribution(generator);
            }
            normalize(a);

            // DEBUG BEGIN
            // print out the hash function
            //std::string hash_function_string = "";
            //for ( int j = 0; j < this->_dimension; ++j){
            //    hash_function_string += ( std::to_string(a[j]) + " " );
            //}
            //husky::LOG_I << "Hash index: " << i;
            //husky::LOG_I << "Hash detail: " << hash_function_string; 
            // DEBUG END

            CosLSHFunction<ItemIdType, ItemElementType> func(a);
            
            // can be improved by swap
            _hashFunctions[i] = func;
        }
    }
    // the below is for sparseVector
    // for sparse vector, assume index is sorted, vector is normalized
    // have been tested, fully correct
    virtual float calDist(
           const vector<pair<int, ItemElementType> > & queryVector,
           const vector<pair<int, ItemElementType> > & itemVector) override{

        typename vector<pair<int, ItemElementType> >::const_iterator qIt = queryVector.begin();
        typename vector<pair<int, ItemElementType> >::const_iterator itemIt = itemVector.begin();

        float cosTheta = 0;
        while(itemIt != itemVector.end() && qIt != queryVector.end() ){
            if( (*itemIt).first < (*qIt).first ){
                itemIt++;
            }else if( (*itemIt).first > (*qIt).first ){
                qIt++;
            }else{
                cosTheta += ( (*itemIt).second )  *  ( (*qIt).second );
                ++itemIt; ++qIt;
            }
        }

        if(cosTheta > 1){
            assert(cosTheta - 1 < 0.001);
            cosTheta = 1.0;
        
        }else if(cosTheta < -1){
            assert(-cosTheta - 1 < 0.001);
            cosTheta = -1.0;
        }

        //return angle in the interval [0, pi] radians
        return acos(cosTheta);
    }

    // this is the same as denseVector except parameter is different
    virtual vector<bool> calSignaturesInBool(
        const vector< pair<int, ItemElementType> > &itemVector) {

        vector<bool> allSignatures;
        allSignatures.resize( _hashFunctions.size() );

        int index = 0;
        for(auto& fun : _hashFunctions) {
            allSignatures[index++] = fun.getQuantization(itemVector);
        }
        return allSignatures;
    }

    // this is the same as denseVector except parameter is different
    // compress bit string into int
    virtual vector< vector<int> > calSigs(
            const vector< pair<int, ItemElementType> > &itemVector) override{

        vector<bool> allSignatures = this->calSignaturesInBool(itemVector);

        // This value is the same as _setBand
        int numOfSignatures = this->_band * (this->_band - 1) / 2;
        // This value is the same as _setRow
        int lengthOfBooleanSignature = this->_row * 2;

        int sizeOfRow = std::ceil( lengthOfBooleanSignature / 32.0 ); 
        vector< vector<int> > signatureInBands;
        signatureInBands.resize( numOfSignatures );
        for (int i = 0 ; i < signatureInBands.size(); ++i) {
            for (int j = 0; j < sizeOfRow; ++j) {
                signatureInBands[i].push_back(0);
            }
        }
        
        int vecIndex;
        int bitSetIndex;
        int intValue;

        for(int i = 0; i < numOfSignatures; ++i) {

            int start = i * lengthOfBooleanSignature;
            vecIndex = 0;
            bitSetIndex = 0;
            intValue = 0;

            //compress bool vector into int vector
            //may have problem since this is not unsigned int
            for(int j = 0; j < lengthOfBooleanSignature; ++j) {
                if(bitSetIndex < 32){
                    intValue = (intValue << 1) + allSignatures[start + j];
                    bitSetIndex++;
                }else{
                    signatureInBands[i][vecIndex++] = intValue;
                    intValue = 0;
                    intValue = (intValue << 1) + allSignatures[start + j];
                    bitSetIndex = 1;
                }
            }
            if(bitSetIndex != 0){
                signatureInBands[i][vecIndex++] = intValue;
            }

            // Debug BEGIN
            //std::string int_string = "";
            //for (int j = 0; j < signatureInBands[i].size(); ++j){
            //    int_string += std::to_string(signatureInBands[i][j]) + " ";
            //}
            //husky::LOG_I << "signature #: " << i << ", detail: " << int_string;
            // Debug END

        }
        return signatureInBands;
    }

    // added in 2016.4.25, to support projection
    vector<float> calProjectionsInBool(
        const vector< pair<int, ItemElementType> >& itemVector) {

        vector<float> allProjections;
        allProjections.resize( _hashFunctions.size() );

        int index = 0;
        for(auto& fun : _hashFunctions) {
            allProjections[index++] = fun.getProjection(itemVector);
        }
        return allProjections;
    }

    virtual vector< vector<float> > calProjs(
        const vector<pair<int, ItemElementType> > &itemVector) override {

        vector<float> allProjections = this->calProjectionsInBool(itemVector);

        vector< vector<float> > projectionsInBands;
        projectionsInBands.resize(_hashFunctions.size());

        for(int i = 0; i < this->_band; ++i) {
            vector<float> vec;
            vec.resize(this->_row);
            int start = i * this->_row;
            for(int j = 0; j < this->_row; ++j) {
                vec[j] = allProjections[start + j];
            }
            projectionsInBands[i].swap(vec);
        }
        return projectionsInBands;
    }

    // // visualization
    // std::string itemAndSignaturesToString(
    //         DenseVector<ItemIdType, ItemElementType>& p,
    //         const vector< vector<int> >& signatures) {
    //
    //         std::string str = "item " + std::to_string(p) + " and its signatures: ";
    //         for(auto& s : signatures){
    //             str += std::to_string(s) + ", ";
    //
    //             //add binary representation
    //             // + "or " 
    //             // vector<bool> sig;
    //             // while(){
    //             // }
    //             // str += ", ";
    //         }
    //         str += ";";
    // }
    //
    // // visualization
    // std::string LSHFunctionsToString() {
    //
    //     std::string parametersLog = "\n\nworker" +
    //                                 std::to_string(get_worker().id) + ": ";
    //
    //     parametersLog += "row: " + std::to_string(row) +
    //                      ", band: " + std::to_string(band) +
    //                      ", dimensions: " + std::to_string(dimension);
    //
    //     int index = 0;
    //     std::string hashFunctionStr = "hash function";
    //     //get only the first function
    //     for(int i = 0; i < this->getRows(); ++i)
    //     parametersLog += "\n" + hashFunctionStr + std::to_string(i) +  
    //         ": " +_hashFunctions[i].toString();
    //     //get all the functions
    //     // for(auto &h : _hashFunctions) {
    //     //     parametersLog += "\n" + hashFunctionStr + std::to_string(index++) +  
    //     //         ": " +h.toString();
    //     // }
    //     parametersLog += "\n\n";
    //     return parametersLog;
    // }
};

} // namespace losha
} // namespace husky
