#include "e2lsh.hpp"
// To parse SIFT1B
typedef int ItemIdType;
typedef char ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
const int idBytes_sift1b = 4;
const int dimension_sift1b = 128;
const int elementBytes_sift1b = sizeof(ItemElementType);

// input format: id f1 f2 
void setItemSIFT1B(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    memcpy(&itemId, &line[0], idBytes_sift1b);
    itemVector.resize(dimension_sift1b);
    int startIdx = idBytes_sift1b;
    for (int i = 0; i < dimension_sift1b; ++i) {
        memcpy(&itemVector[i], &line[startIdx], elementBytes_sift1b);
        startIdx += elementBytes_sift1b;
    } 
}

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query1B;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item1B;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket1B;

const int BytesPerVector = idBytes_sift1b + dimension_sift1b * elementBytes_sift1b;
