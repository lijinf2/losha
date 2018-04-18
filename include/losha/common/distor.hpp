#pragma once
#include <vector>
#include <cmath>
#include "losha/common/dotproduct.hpp"
#include "losha/common/algebra.hpp"
using namespace std;

namespace husky {
namespace losha {

float calE2Dist(
        const std::vector<float> & queryVector,
        const std::vector<float> & itemVector) {

    typename std::vector<float>::const_iterator qIt = queryVector.begin();
    typename std::vector<float>::const_iterator myIt = itemVector.begin();

    float distance = 0;
    while (myIt != itemVector.end() && qIt != queryVector.end()) {
        distance += (*myIt - *qIt) * (*myIt - *qIt);
        ++myIt; ++qIt;
    }
    return sqrt(distance);
}

float calAngularDist(
        const std::vector<float> & queryVector,
        const std::vector<float> & itemVector,
        bool unitNorm = false) {

    float product = dotProduct(queryVector, itemVector);

    if (!unitNorm && product != 0) {
        product /= calL2Norm(queryVector);
        product /= calL2Norm(itemVector);
    } 
    if (product > 1) {
        product = 1;
    }
    else if (product < -1) {
        product = -1;
    }

    return acos(product);
}

float calAngularDist(
        const std::vector<std::pair<int, float>> & queryVector,
        const std::vector<std::pair<int, float>> & itemVector,
        bool unitNorm = false) {

    float product = dotProduct(queryVector, itemVector);

    if (!unitNorm) {
        product /= calL2Norm(queryVector);
        product /= calL2Norm(itemVector);
    } 

    return acos(product);
}
}
}
