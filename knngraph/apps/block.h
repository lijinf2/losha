#pragma once
#include <vector>
#include "core/engine.hpp"
using std::vector;

namespace husky {
namespace losha {

class AdjList {
public:
    int vertexId;
    int cc;
    vector<int> nbIds;
    vector<float> dists; 
    friend husky::BinStream& operator<<(husky::BinStream& stream, const AdjList& adjlist) {
        stream <<  adjlist.vertexId << adjlist.cc << adjlist.nbIds << adjlist.dists;
        return stream;
    }
    friend husky::BinStream& operator>>(husky::BinStream& stream, AdjList& adjlist) {
        stream >>  adjlist.vertexId >> adjlist.cc >> adjlist.nbIds >> adjlist.dists;
        return stream;
    }
};

template<typename FeatureType>
class Block{
public:
    using KeyT = int;
    KeyT _bid;
    explicit Block(const KeyT& id) : _bid(id) {};
    Block() = default;
    const KeyT id() const {return _bid;};

    vector<AdjList> adjLists;

    friend husky::BinStream& operator<<(husky::BinStream& stream, const Block<FeatureType>& v) {
        stream << v._bid << v.adjLists;
        return stream;
    }
    friend husky::BinStream& operator>>(husky::BinStream& stream, Block<FeatureType>& v) {
        stream >> v._bid >> v.adjLists;
        return stream;
    }
};
}
}
