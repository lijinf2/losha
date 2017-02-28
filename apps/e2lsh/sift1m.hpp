#include "e2lsh.hpp"
// To parse SIFT1M
typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
// input format: id f1 f2 
const int idBytes_sift1m = 4;
const int dimension_sift1m = 128;
const int elementBytes_sift1m = sizeof(ItemElementType);
void setItemSIFT1M(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    memcpy(&itemId, &line[0], idBytes_sift1m);
    itemVector.resize(dimension_sift1m);
    int startIdx = idBytes_sift1m;
    for (int i = 0; i < dimension_sift1m; ++i) {
        memcpy(&itemVector[i], &line[startIdx], elementBytes_sift1m);
        startIdx += elementBytes_sift1m;
    }
}

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query1M;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item1M;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket1M;

const int BytesPerVector = idBytes_sift1m + dimension_sift1m * elementBytes_sift1m;
