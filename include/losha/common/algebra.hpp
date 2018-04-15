#pragma once
#include <vector>
#include <cmath>
namespace husky{
namespace losha {

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

