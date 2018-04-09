// LSHFactory could be expensive for high dimensional data. E.g., 500k tweet, 100 band and 16 row, 24 threads, totally 5 x 10^8 * 24 = 10G

#pragma once
#include <unordered_map>
#include <vector>

#include "densevector.hpp"

using std::vector;

namespace husky {
namespace losha {

template<typename ItemIdType, typename ItemElementType>
class LSHFactory {
public:
    int _band;
    int _row;
    int _dimension;
    std::unordered_map<ItemIdType, std::vector<ItemElementType>> _idToQueryVector;

    // three most important virtual functions, calDist, oldCalSigs and calProjs
    virtual float calDist(
        const vector<ItemElementType> & query,
        const vector<ItemElementType> & item) = 0;

    virtual vector< vector<int> > calSigs( 
        const vector<ItemElementType> &itemVector) = 0;

    virtual vector< vector<float> > calProjs(
        const vector<ItemElementType> &itemVector) {

        // should return an error since it only belongs to E2LSH
        vector< vector<float> > zero;
        return zero;
    }

    // wrapper for DenseVector
    inline float calDist(
        const DenseVector<ItemIdType, ItemElementType> & query,
        const DenseVector<ItemIdType, ItemElementType> & item) {
        return calDist(query.getItemVector(), item.getItemVector() );
    }

    // wrapper for DenseVector
    inline float calDist(
        const vector<ItemElementType> & queryVector,
        const DenseVector<ItemIdType, ItemElementType> & item) {
        return calDist(queryVector, item.getItemVector() );
    }

    // wrapper for DenseVector
    inline float calDist(
        const DenseVector<ItemIdType, ItemElementType> & query,
        vector<ItemElementType> & itemVector) {
        return calDist(query.getItemVector(), itemVector );
    }

    // wrapper for DenseVector
    inline float calSigs(
        const DenseVector<ItemIdType, ItemElementType> & item) {
        return calSigs(item.getItemVector() );
    }
    
    // wrapper for DenseVector
    inline float calProjs(
        const DenseVector<ItemIdType, ItemElementType> & item) {
        return calProjs(item.getItemVector() );
    }

    // wrapper to add table index 
    // directly get buckets for an object
    vector< vector<int> > calItemBuckets( const vector<ItemElementType>& itemVector) {
        vector< vector<int> > sigInBands = this->calSigs(itemVector);

        vector< vector<int> >::iterator it = sigInBands.begin();
        int table = 0;
        for (; it != sigInBands.end(); ++it) {
            (*it).push_back(table++);
        }
        return sigInBands;
    }

    // wrapper for DenseVector
    vector< vector<int> > calItemBuckets(
        const DenseVector<ItemIdType, ItemElementType>& p) {
        return calItemBuckets(p.getItemVector());
    }

    inline int getBand() {
        return _band;
    }

    inline int getRow() {
        return _row;
    }

    inline int getDimension() {
        return _dimension;
    }

    // Fetch broadcasted queries into _idToQueryVector
    inline void insertQueryVector(int qid, const std::vector<ItemElementType>& qvec) {
        if (_idToQueryVector.find(qid) != _idToQueryVector.end()) {
            ASSERT_MSG(0, "query already exists");
        }
        _idToQueryVector[qid] = qvec;
    }

    const std::vector<ItemElementType>& getQueryVector(ItemIdType qid) {
        ASSERT_MSG(_idToQueryVector.size() != 0, "All queries are processed");
        if (_idToQueryVector.find(qid) == _idToQueryVector.end()) {
            ASSERT_MSG(0, "cannot find query");
        }
        return _idToQueryVector[qid];
    }

    const std::unordered_map<ItemIdType, std::vector<ItemElementType>>& getAllQueries() {
        return _idToQueryVector;
    }
    // handle aggregator variable

    // /* general functions*/
    // /*
    //  * get buckets from signatures of a point, should work for both denseVector and sparseVector
    //  * usually add table id in the end
    //  * algorithm: add table id in the end of each signature*/
    // // attention: very low efficient since copy the signaturesInBands
    // vector< vector<int> > createBucketsFromSignatures(
    //     const vector< vector<int> >& signaturesInBands) {
    //     vector< vector<int> > myBuckets;
    //
    //     // last value denotes which table
    //     for (int i = 0; i < signaturesInBands.size(); ++i) {
    //         vector<int> bk = signaturesInBands[i];
    //         bk.push_back(i);
    //         myBuckets.push_back(bk);
    //     }
    //
    //     return myBuckets;
    // }
    //
    //
    // vector<vector<int>> oldCalSigs(
    //     const DenseVector<ItemIdType, ItemElementType>& p) {
    //     return calItemBuckets(p);
    // }

    // virtual std::string itemAndSignaturesToString(
    //     const DenseVector<ItemIdType, ItemElementType>& p,
    //     const vector< vector<int> >& signatures) {
    //     std::string str = "item " + std::to_string(p) + " and its signatures: ";
    //     for (auto& s : signatures) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     return str;
    // }
    //
    // virtual std::string itemAndBucketsToString(
    //     const DenseVector<ItemIdType, ItemElementType> &p,
    //     const vector< vector<int> >& buckets) {
    //     std::string str = "item " + std::to_string(p) + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     return str;
    // }
    //
    // // report hash functions generated
    // virtual std::string LSHFunctionsToString() {
    // }
    //
    // // key function deprecated and replaced by itemAndSignaturesToString
    // static void reportItemAndSignatures(
    //     DenseVector<ItemIdType, ItemElementType>& p,
    //     const vector< vector<int> >& signatures) {
    //     std::string str = "item " + p.toString() + " and its signatures: ";
    //     for (auto& s : signatures) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // static void reportItemAndSignatures(
    //     DenseVector<ItemIdType, ItemElementType>& p,
    //     const vector<int>& signatures) {
    //     std::string str = "item " + std::to_string(p) + " and its signatures: ";
    //     for (auto& s : signatures) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // // for bucket with id type unsigned int
    // static void reportItemAndBuckets(DenseVector<ItemIdType, ItemElementType> &p,
    //                                  const std::set<unsigned int>& buckets) {
    //     std::string str = "item " + p.toString() + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // static void reportItemAndBuckets(DenseVector<ItemIdType, ItemElementType> &p,
    //                                  const vector<unsigned int>& buckets) {
    //     std::string str = "item " + p.toString() + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // // for bucket with id type vector<int>
    // static void reportItemAndBuckets(DenseVector<ItemIdType, ItemElementType> &p,
    //                                  const std::set< vector<int> >& buckets) {
    //     std::string str = "item " + p.toString() + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // static void reportItemAndBuckets(DenseVector<ItemIdType, ItemElementType> &p,
    //                                  const vector< vector<int> >& buckets) {
    //     // std::string str = "item " + p.toString() + " and its buckets: ";
    //     std::string str = "item " + std::to_string(p) + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    //
    // // for sparse vector
    // static void reportItemAndBuckets(DenseVector<ItemIdType, std::pair<int, ItemElementType> > &p,
    //                                  const vector< vector<int> >& buckets) {
    //     std::string str = "item " + std::to_string(p) + " and its buckets: ";
    //     for (auto& s : buckets) {
    //         str += std::to_string(s) + ", ";
    //     }
    //     str += ";";
    //     husky::base::log_msg(str);
    // }
    // //optional: support sparseVector
    // virtual vector< vector<int> > calSignatureInBands(
    //     const DenseVector<ItemIdType, std::pair<int, ItemElementType> > &p) {
    //
    // }
    //
    // virtual float calDist(
    //     const DenseVector<ItemIdType, std::pair<int, ItemElementType> > & query,
    //     const DenseVector<ItemIdType, std::pair<int, ItemElementType> > & item) {
    //
    // }
    //
    // virtual vector< vector<float> > calProjs(
    //     const DenseVector<ItemIdType, std::pair<int, ItemElementType> >& p) {
    //     //should return an error since it only belongs to E2LSH
    //     vector< vector<float> > zero;
    //     return zero;
    // }

    // int bands;  // number of bands
    // int rows;  // number of rows in a band
    // int dimension;  // number of features

    // int getBands() {
    //     return this->bands;
    // }
    // int getRows() {
    //     return this->rows;
    // }
    // int getDimensions() {
    //     return this->dimension;
    // }
};

} // namespace losha
} // namespace husky
