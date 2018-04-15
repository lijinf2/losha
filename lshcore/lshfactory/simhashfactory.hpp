#pragma once
#include <map>
#include <string>
#include <vector>

#include "base/log.hpp"

#include "losha/common/distor.hpp"
#include "lshcore/lshfactory/simhashfunction.hpp"
#include "lshcore/lshfactory.hpp"
#include "lshcore/lshutils.hpp"

#define PI 3.141592654
template<typename ItemIdType, typename ItemElementType>
class SimHashFactory:
    public LSHFactory<ItemIdType, ItemElementType> {
public:
    std::vector< SimHashFunction<ItemIdType, ItemElementType> >  hashFunctions;

    void initialize(int bands, int rows, int dimension, int seed = 0) {
        this->_band = bands;
        this->_row = rows;
        this->_dimension = dimension;

        generateSimHashFunctions(seed);
    }

    // generate hash functions
    void generateSimHashFunctions(int seed) {
        std::default_random_engine generator(seed);
        std::normal_distribution<float> distribution(0.0, 1.0);

        int numFunctions = this->_band * this->_row;


        for (int i = 0; i < numFunctions; ++i) {
            std::vector<float> parameters(this->_dimension);
            for (int j = 0; j < this->_dimension; ++j) {
                parameters[j] = distribution(generator);
            }
            normalize(parameters);
            SimHashFunction<ItemIdType, ItemElementType> func(parameters);
            hashFunctions.emplace_back(func);
        }
    }

    // compress into std vector<int>
    std::vector< std::vector<int> > calSigs(
        const vector<ItemElementType> &p) const override {
        std::vector<bool> allSignatures = this->calSignaturesInBool(p);

        int numRows = this->getRow();
        int numBands =  this->getBand();
        std::vector< std::vector<int> > signatureInBands;

        int intValue;
        for (int i = 0; i < numBands; ++i) {
            int start = i * numRows;
            signatureInBands.push_back(boolsToInts(allSignatures, start, numRows));
        }
        return signatureInBands;
    }

    // for denseVector, NO ASSUMPTION a * b / |a| / |b|
    virtual float calDist(
           const std::vector<ItemElementType> & queryVector,
           const std::vector<ItemElementType> & itemVector) const override {

        return calAngularDist(queryVector, itemVector);
    }

private:
    // for denseVector
    std::vector<bool> calSignaturesInBool(
        const vector<ItemElementType>& p) const {
        std::vector<bool> allSignatures;
        allSignatures.resize(this->hashFunctions.size());

        int index = 0;
        for (auto& fun : this->hashFunctions) {
            allSignatures[index++] = fun.getBucket(p);
        }
        return allSignatures;
    }

    std::vector<int> boolsToInts(
        const vector<bool>& array, int startIdx, int numBools) const {
        // compress bool vector into int vector
        vector<int> result;
        int iter = startIdx;
        while (iter < startIdx + numBools) {
            int value = 0;
            for (int i = 0; i < 32; ++i) {
                value <<= 1;
                value += array[iter];
                iter++;
                if (iter == startIdx + numBools)
                    break;
            }
            result.push_back(value);
        }
        return result;
    }
};
