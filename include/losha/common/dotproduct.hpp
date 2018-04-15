#pragma once
#include <vector>

namespace husky {
namespace losha {

template<typename ItemElementType>
inline float dotProduct(
    const std::vector<ItemElementType>& a, 
    const std::vector<ItemElementType>& v2) {
    assert(a.size() == v2.size());
    float product = 0;
    for (int i = 0; i < a.size(); ++i) {
        product += a[i] * v2[i];
    }
    return product;
}

// for sparseVector
template<typename ItemElementType>
inline float dotProduct(
    const std::vector<ItemElementType>& a, 
    const std::vector< std::pair<int, ItemElementType> >& v2) {
    float product = 0;
    for (int i = 0; i < v2.size(); ++i) {
        product += a[ v2[i].first ] * v2[i].second;
    }
    return product;
}

}
}
