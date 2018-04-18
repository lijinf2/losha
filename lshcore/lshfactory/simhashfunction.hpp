#pragma once
#include <math.h>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "losha/common/dotproduct.hpp"
using std::pair;
using std::vector;

namespace husky {
namespace losha {

template<typename ItemElementType>
class SimHashFunction {
public:
    SimHashFunction() {}
    explicit SimHashFunction(std::vector<float>& a) {
        _a.swap(a);
        if(_a.capacity() != _a.size())
            _a.shrink_to_fit();
    }

    // for both denseVector and sparse vector
    inline float getProjection(
        const std::vector<ItemElementType>& itemVector) const {
        return dotProduct(_a, itemVector);
    }

    // for bot denseVector and sparse vector
    bool getBucket(
        const std::vector<ItemElementType>& itemVector) const {
        if (this->getProjection(itemVector) >= 0)
            return true;
        else
            return false;
    }

private:
    std::vector<float> _a;
};

} // namespace losha
} // namespace husky
