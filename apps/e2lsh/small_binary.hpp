#include "e2lsh.hpp"
// To parse small dataset (binary)
typedef int ItemIdType;
typedef int ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
const int idBytes_ssb = sizeof(ItemIdType);
const int dimension_ssb = 5;
const int elementBytes_ssb = sizeof(ItemElementType);

// input format: id f1 f2 f3 f4 f5 
void setItemSmallBinary(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    memcpy(&itemId, &line[0], idBytes_ssb);
    itemVector.resize(dimension_ssb);
    int startIdx = idBytes_ssb;
    for (int i = 0; i < dimension_ssb; ++i) {
        memcpy(&itemVector[i], &line[startIdx], elementBytes_ssb);
        startIdx += elementBytes_ssb;
    } 
}

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query1B;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item1B;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket1B;

const int BytesPerVector = idBytes_ssb + dimension_ssb * elementBytes_ssb;
