#pragma once
#include <vector>
#include <cmath>
#include "losha/common/dotproduct.hpp"
#include "losha/common/algebra.hpp"
using namespace std;

namespace husky {
namespace losha {

template<typename ItemElementType>
float calE2Dist(
        const std::vector<ItemElementType> & queryVector,
        const std::vector<ItemElementType> & itemVector) {

    typename std::vector<ItemElementType>::const_iterator qIt = queryVector.begin();
    typename std::vector<ItemElementType>::const_iterator myIt = itemVector.begin();

    float distance = 0;
    while (myIt != itemVector.end() && qIt != queryVector.end()) {
        distance += (*myIt - *qIt) * (*myIt - *qIt);
        ++myIt; ++qIt;
    }
    return sqrt(distance);
}

template<typename ItemElementType>
float calAngularDist(
        const std::vector<ItemElementType> & queryVector,
        const std::vector<ItemElementType> & itemVector,
        bool unitNorm = false) {

    float product = dotProduct(queryVector, itemVector);

    if (!unitNorm && product != 0) {
        product /= calL2Norm(queryVector);
        product /= calL2Norm(itemVector);
    } 
    if (product > 1 || fabs(product - 1) < 0.001) {
        product = 1;
    }
    else if (product < -1 || fabs(product - (-1))  < 0.001) {
        product = -1;
    }

    return acos(product);
}
}
}
