#pragma once
#include <vector>
#include <cmath>
#include "losha/common/dotproduct.hpp"
#include "losha/common/algebra.hpp"
using namespace std;

namespace husky {
namespace losha {

inline float calSquareE2Dist(
        const std::vector<float> & queryVector,
        const std::vector<float> & itemVector) {

    assert(queryVector.size() == itemVector.size());
    float distance = 0;
    for (int i = 0; i < queryVector.size(); ++i) {
        distance += (queryVector[i] - itemVector[i]) * (queryVector[i] - itemVector[i]);
    }
    return distance;
}

inline float calE2Dist(
        const std::vector<float> & queryVector,
        const std::vector<float> & itemVector) {

    return sqrt(calSquareE2Dist(queryVector, itemVector));
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
