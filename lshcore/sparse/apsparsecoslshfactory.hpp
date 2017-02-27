#pragma once
#include <vector>
#include "lshcore/densevector.hpp"
#include "lshcore/lshutils.hpp"
#include "lshcore/lshfactory.hpp"
#include "lshcore/sparse/sparsecoslshfactory.hpp"

using std::vector;
using std::pair;

// Users must guarantee the above formula stands
// AProw = row / 2, APSparseCosLSHFactory requires concatenate two row / 2 sequences to for one signature
// band = (APBand * (APBand - 1)) / 2 
// APSparseCosLSHFacotry will generate AProw and APBand
namespace husky {
namespace losha {
    
template<typename ItemIdType, typename ItemElementType>
class APSparseCosLSHFactory:
    public SparseCosLSHFactory<ItemIdType, ItemElementType> {

public:
    // default _band and _row are calculated by setBand and setRow
    // _Row = _setRow / 2, and _setBand = _band * (_band - 1) / 2
    int _setBand;
    int _setRow;
    // vector< CosLSHFunction<ItemIdType, ItemElementType> >  _hashFunctions;

    /* Set parameters
     * */
    // void initialize() {
    //     initialize(Husky::Context::get_params());
    // }

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
        // if(get_worker().id == 0){
        //     Husky::log_msg("band: ");
        // }
        assert(m != 0);

        // this->initialize(m, row / 2, dimension, seed);
        this->_band = m;
        this->_row = row / 2;
        this->_dimension = dimension;
        this->_seed = seed;

        this->generateFunctions();
    }

    // the below is for sparseVector

    // this is the same as denseVector except parameter is different
    virtual vector<bool> calSignaturesInBool(
        const vector < pair<int, ItemElementType> > &itemVector) override {


        vector<bool> halfSig;
        halfSig.resize( this->_hashFunctions.size() );
        int index = 0;
        for(auto& fun : this->_hashFunctions) {
            halfSig.at(index++) = fun.getQuantization(itemVector);
        }

        husky::LOG_I << "invoke calSignatureInBool " << std::endl;
        //this->_row = getRows / 2; this->_band * (this->_band - 1) == getBands()
        vector<bool> allSignatures;
        allSignatures.resize( this->_setBand * this->_setRow);

        index = 0;
        int leftIndex;
        int rightIndex;
        for(int l = 0; l < this->_band - 1; ++l){
            leftIndex = l * this->_row;
            for(int r = l + 1; r < this->_band; ++r){
                rightIndex = r * this->_row;
                // first half
                for(int it1 = 0; it1 < this->_row; ++it1){
                    // allSignatures[index++] = halfSig[leftIndex + it1];
                    allSignatures.at(index++) = halfSig.at(leftIndex + it1);
                }
                // second half
                for(int it2 = 0; it2 < this->_row; ++it2){
                    allSignatures.at(index++) = halfSig.at(rightIndex + it2);
                }
            }
        }

        assert( allSignatures.size() == this->_setBand * this->_setRow);

        return allSignatures;
    }

    virtual vector< vector<float> > calProjs(
        const vector< pair<int, ItemElementType> > &itemVector) override {

        vector<bool> halfProj;
        halfProj.resize( this->_hashFunctions.size() );
        int index = 0;
        for(auto& fun : this->_hashFunctions) {
            halfProj[index++] = fun.getProjection(itemVector);
        }

        vector<float> allProjections;
        allProjections.resize( this->_setBand * this->_setRow);

        index = 0;
        int leftIndex;
        int rightIndex;
        for(int l = 0; l < this->_band - 1; ++l){
            leftIndex = l * this->_row;
            for(int r = l + 1; r < this->_band; ++r){
                rightIndex = r * this->_row;
                //first half
                for(int it1 = 0; it1 < this->_row; ++it1){
                    allProjections[index++] = halfProj[leftIndex + it1];
                }
                //second half
                for(int it2 = 0; it2 < this->_row; ++it2){
                    allProjections[index++] = halfProj[rightIndex + it2];
                }
            }
        }

        assert( allProjections.size() == this->_setBand * this->_setRow);

        vector< vector<float> > projectionsInBands;
        vector<float> vec;

        int row = this->_row;
        int band = this->_band;
        vec.resize(row);
        for(int i = 0; i < band; ++i) {
            int start = i * row;
            for(int j = 0; j < row; ++j) {
                vec[j] = allProjections[start + j];
            }
            projectionsInBands.push_back(vec);
        }
        return projectionsInBands;
    }
};

} // namespace losha
} // namespace husky
