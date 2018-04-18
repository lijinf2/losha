#pragma once
#include <vector>
#include <cmath>
#include <utility>
namespace husky{
namespace losha {

template<typename T>
float calL2Norm(const std::vector<std::pair<int, T>>& vector) {
    float dist = 0;
    for (auto& p : vector) {
        dist += p.second * p.second;
    }
    return sqrt(dist);
}

template<typename T>
float calL2Norm(const std::vector<T>& vector) {
    float dist = 0;
    for (auto& e : vector) {
        dist += e * e;
    }
    return sqrt(dist);
}
}
}

