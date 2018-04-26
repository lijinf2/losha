#pragma once
#include <vector>
using std::vector;

class AdjList {
public:
    int vertexId;
    int cc;
    vector<int> neighbors;
    vector<float> dists; 
};

template<typename FeatureType>
class Block{
public:
    using KeyT = int;
    KeyT _bid;
    explicit Block(const KeyT& id) : _bid(id) {};
    const KeyT id() const {return _bid};

    vector<AdjList> adjLists;

};
