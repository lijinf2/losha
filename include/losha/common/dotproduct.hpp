#pragma once
#include <vector>
#include <utility>
#include <cassert>
using std::vector;
using std::pair;

namespace husky {
namespace losha {

inline float dotProduct(
    const std::vector<float>& a, 
    const std::vector<float>& v2) {
    assert(a.size() == v2.size());
    float product = 0;
    for (int i = 0; i < a.size(); ++i) {
        product += a[i] * v2[i];
    }
    return product;
}

// for sparseVector
inline float dotProduct(
    const std::vector<float>& a, 
    const std::vector< std::pair<int, float> >& v2) {
    float product = 0;
    for (int i = 0; i < v2.size(); ++i) {
        product += a[ v2[i].first ] * v2[i].second;
    }
    return product;
}

// for sparseVector
inline float dotProduct(
    const std::vector<std::pair<int, float>>& queryVector, 
    const std::vector<std::pair<int, float>>& itemVector) {

    typename vector<pair<int, float> >::const_iterator qIt = queryVector.begin();
    typename vector<pair<int, float> >::const_iterator itemIt = itemVector.begin();

    float product = 0;
    while(itemIt != itemVector.end() && qIt != queryVector.end() ){
        if( (*itemIt).first < (*qIt).first ){
            itemIt++;
        }else if( (*itemIt).first > (*qIt).first ){
            qIt++;
        }else{
            product += ( (*itemIt).second )  *  ( (*qIt).second );
            ++itemIt; ++qIt;
        }
    }
    return product;
}
}
}
