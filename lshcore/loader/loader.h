#include <vector>
#include <string>
#include "boost/tokenizer.hpp"
#include "lshcore/lshutils.cpp"
using std::vector;
void parseIdFvecs(
    boost::string_ref& line,
    int & itemId,
    std::vector<float>& itemVector) {

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
        memcpy(&itemVector[i], &line[startIdx], 4);
        startIdx += 4;
    } 
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
