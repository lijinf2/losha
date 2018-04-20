#pragma once
#include <vector>
#include <utility>
#include "lshcore/densevector.hpp"
#include "lshcore/lshutils.hpp"
#include "lshcore/lshfactory.hpp"
#include "lshcore/lshfactory/simhashfactory.hpp"

using std::vector;
using std::pair;

// Users must guarantee the above formula stands
// AProw = row / 2, APSparseCosLSHFactory requires concatenate two row / 2 sequences to for one signature
// band = (APBand * (APBand - 1)) / 2 
// APSparseCosLSHFacotry will generate AProw and APBand
namespace husky {
namespace losha {
    
template<typename ItemIdType, typename ItemElementType>
class APSimHashFactory:
    public SimHashFactory<ItemIdType, ItemElementType> {

public:
    // default _band and _row are calculated by setBand and setRow
    // _Row = _setRow / 2, and _setBand = _band * (_band - 1) / 2
    int _setBand;
    int _setRow;

    void initialize( int band, int row, int dimension, int seed = 0) {

        _setBand = band;
        _setRow = row;
        assert(row % 2 == 0);

        //factor band = m(m-1)/2;
        int m = 0;
        for(int i = ceil( sqrt( 2 * band + 1) + 1); i > 0; --i ){
            if(i * (i - 1) / 2 == band){
                m = i;
                break;
            }
        }
        assert(m != 0);

        this->_band = m;
        this->_row = row / 2;
        this->_dimension = dimension;

        this->generateSimHashFunctions(seed);
    }

    std::vector< std::vector<int> > calSigs(
        const vector<ItemElementType> &p) const override {
        std::vector<bool> bools = this->calSignaturesInBool(p);

        // expand allSignatures by all-pair lsh
        auto allBools = concat(bools, this->getRow(), this->getBand());
        assert(allBools.size() == _setBand * _setRow);
        return this->boolsToSigs(allBools, _setRow, _setBand);
    }

protected:
    static std::vector<bool> concat(
        const std::vector<bool>& origin, int numRows, int numBands) {

        int size = numRows * 2 * (numBands - 1) * numBands / 2;
        std::vector<bool> allBools(size);

        int index = 0;
        for(int l = 0; l < numBands - 1; ++l){
            int leftIndex = l * numRows;
            for(int r = l + 1; r < numBands; ++r){
                int rightIndex = r * numRows;

                // first half
                for(int it1 = 0; it1 < numRows; ++it1){
                    allBools[index++] = origin[leftIndex + it1];
                }
                // second half
                for(int it2 = 0; it2 < numRows; ++it2){
                    allBools[index++] = origin[rightIndex + it2];
                }
            }
        }

        return allBools;
    }
};

template<typename ItemIdType, typename ItemElementType>
class APSparseSimHashFactory:
    public APSimHashFactory<ItemIdType, pair<int, ItemElementType>> {
};

} // namespace losha
} // namespace husky
