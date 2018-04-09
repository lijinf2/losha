#include <vector>
#include <string>
#include <boost/utility/string_ref.hpp>
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
