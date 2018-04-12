#include <vector>
#include <cmath>
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
}
}
