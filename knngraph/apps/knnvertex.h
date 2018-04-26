#pragma once
#include "lshcore/densevector.hpp"
#include <utility>
#include <vector>
#include <limits>
#include <random>
#include <unordered_set>
using std::pair;
using std::vector;
namespace husky {
namespace losha {

// Optimization: separate data and knn for better cache locality
template<typename ItemElementType>   
class KNNVertex : public DenseVector<int, ItemElementType>{
public:
    vector<pair<int, float>> trueKNN;
    vector<pair<int, float>> foundKNN;

    explicit KNNVertex(
        const typename KNNVertex::KeyT& id) : DenseVector<int, ItemElementType>(id) {};
    KNNVertex() : DenseVector<int, ItemElementType>(){}

    void setTrueKNN(const vector<pair<int, float>>& knn) {
        this->trueKNN = knn;
    }

    void initFoundKNN(int maxItemId) {
        std::default_random_engine generator(this->getItemId());
        std::uniform_int_distribution<unsigned> distribution(0, maxItemId - 1);

        std::unordered_set<unsigned> inserted;
        float maxDist = std::numeric_limits<float>::max();

        while(foundKNN.size() < trueKNN.size()) {
            unsigned genId = distribution(generator);
            if (genId != this->getItemId()
                && inserted.find(genId) == inserted.end()) {
                inserted.insert(genId);
                foundKNN.emplace_back(std::make_pair(genId, maxDist));
            }
        }
    }

    vector<int> getNeighbors() {
        vector<int> nbs(foundKNN.size());
        for (int i = 0; i < nbs.size(); ++i) {
            nbs[i] = foundKNN[i].first;
        }
        return nbs;
    }
};
}
}
