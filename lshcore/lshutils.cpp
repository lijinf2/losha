#include <string>
#include <utility>
#include <vector>

#include "lshcore/lshutils.hpp"

namespace husky {
namespace losha {

void normalize(std::vector<float> &vec) {
    assert(vec.size() != 0);
    float sum = 0;
    for (auto& v : vec) {
        sum += v * v;
    }
    sum = sqrt(sum);
    assert(fabs(sum - 0) > 0.0000000001);
    for (int i = 0; i < vec.size(); ++i) {
        vec[i] /= sum;
    }
}

void normalize(std::vector<std::pair<int, float> > &vec) {
    assert(vec.size() != 0);
    float sum = 0;
    for (auto& v : vec) {
        sum += v.second * v.second;
    }
    sum = sqrt(sum);
    assert(fabs(sum - 0) > 0.0000000001);
    for (int i = 0; i < vec.size(); ++i) {
        vec[i].second /= sum;
    }
}

std::pair<int, float> lshStofPair(const std::string& pairStr) {
    int splitter = pairStr.find(':');
    int index = std::stoi(pairStr.substr(0, splitter));
    float value = std::stof(pairStr.substr(splitter+1));
    return std::pair<int, float>(index, value);
}

} // namespace losha
} // namespace husky

namespace std {

std::string to_string(const std::string& str) {
    return str;
}

}  // namespace std
