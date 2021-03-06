#pragma once
#include <vector>
#include <string>
#include "boost/tokenizer.hpp"
#include "lshcore/lshutils.cpp"
#include "lshcore/densevector.hpp"
using std::vector;
using std::string;

namespace husky {
namespace losha {
template<typename ItemElementType>
void parseIdFvecs(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    // item id
    memcpy(&itemId, &line[0], 4);

    // dimensionality
    int dimension = -1;
    memcpy(&dimension, &line[4], 4);

    // load fvecs
    itemVector.resize(dimension);
    int startIdx = 8;
    for (int i = 0; i < dimension; ++i) {
        memcpy(&itemVector[i], &line[startIdx], sizeof(ItemElementType));
        startIdx += sizeof(ItemElementType);
    } 
}

template<typename ObjType, typename ItemIdType, typename ItemElementType>
auto item_loader(
    husky::PushChannel< vector<ItemElementType>, ObjType> &ch,
    void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&)) {

    auto parse_lambda = [&ch, &setItem]
    (boost::string_ref & line) {
        try {
            ItemIdType itemId;
            vector<ItemElementType> itemVector;
            setItem(line, itemId, itemVector);

            ch.push(itemVector, itemId);
        } catch(std::exception e) {
            assert("bucket_parser error");
        }
    };
    return parse_lambda;
}


void parseIdLibsvm(
    boost::string_ref& line,
    int& itemId,
    std::vector<std::pair<int, float> >& itemVector) {

    assert(line.size() != 0);

    boost::char_separator<char> sep(" \t");
    boost::tokenizer<boost::char_separator<char>> tok(line, sep);

    //optimize to use dimension to avoid push back set lshStof
    bool setId = false;
    for(auto &w : tok) {
        if(!setId) {
            itemId = std::stoi(w);
            setId = true;
        } else itemVector.push_back(husky::losha::lshStoPair(w));
    }
    assert (itemVector.size() != 0);
}
}
}
