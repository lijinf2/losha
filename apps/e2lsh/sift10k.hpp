#include "e2lsh.hpp"
#include <iostream>
// To parse SIFT10K
typedef int ItemIdType;
typedef float ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
const int idBytes_sift10K = sizeof(ItemIdType); //int
const int dimension_sift10K = 128;
const int elementBytes_sift10K = sizeof(ItemElementType);

// input format: id f1 f2 
void setItemSIFT10K(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    memcpy(&itemId, &line[0], idBytes_sift10K);
    itemVector.resize(dimension_sift10K);
    int startIdx = idBytes_sift10K;

    std::cout<<itemId<<std::endl;
    for (int i = 0; i < dimension_sift10K; ++i) {
        memcpy(&itemVector[i], &line[startIdx], elementBytes_sift10K);
        std::cout << itemVector[i] << " ";
        startIdx += elementBytes_sift10K;
    }

    std::cout<<std::endl;
    std::cout<<std::endl;
}

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query10K;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item10K;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket10K;

const int BytesPerVector = idBytes_sift10K + dimension_sift10K * elementBytes_sift10K;
