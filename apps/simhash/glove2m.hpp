#include "simhash.hpp"
#include <math.h>

typedef int ItemIdType;
typedef std::pair<int, float> ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, float> AnswerMsg;
const int idBytes = sizeof(int);
const int dimension_glove = 300;
const int elementBytes = sizeof(float);
const int BytesPerVector = idBytes + dimension_glove * elementBytes;

// input format : id f1 f2 f3 ...
void setItem(
    boost::string_ref& line,
    int & itemId,
    std::vector<std::pair<int, float> >& itemVector) {

    assert(line.size() != 0);

    // Read input
    memcpy(&itemId, &line[0], idBytes);
    itemVector.resize(dimension_glove);
    int elementHead = idBytes;
    float norm_square = 0;
    for ( int i = 0; i < dimension_glove; ++i) {
        itemVector[i].first = i;
        memcpy(&itemVector[i].second, &line[elementHead], elementBytes);
        elementHead += elementBytes; 
        norm_square += itemVector[i].second * itemVector[i].second;
    }

    // Normalize vector
    float norm = sqrt(norm_square);
    for ( int i = 0; i < dimension_glove; ++i) {
        itemVector[i].second = itemVector[i].second / norm;
    }

    // DEBUG: print out the vector ID
    //husky::LOG_I << "\n Vector ID: " << itemId << "\n";

    // DEBUG: print out the sorted normalized vecter
    //std::string temp = std::to_string(itemId);
    //for ( std::vector< std::pair<int, float> >::iterator it = itemVector.begin(); it != itemVector.end(); ++it)
    //{
    //    temp += " " + std::to_string( (*it).first ) + ":" + std::to_string( (*it).second );
    //}
    //husky::LOG_I << temp << "\n";
    //std::exit(0);

}

typedef SimhashQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashQuery;
typedef SimhashItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashItem;
typedef SimhashBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashBucket;
