#include "simhash.hpp"
#include "boost/tokenizer.hpp"
#include <utility>
#include <vector>
#include <string>

typedef int ItemIdType;
typedef std::pair<int, float> ItemElementType;
typedef ItemIdType QueryMsg;
typedef pair<ItemIdType, float> AnswerMsg;

// input format : id f1:v1 f2:v2 f3:v3 ...
void setItem(
    boost::string_ref& line,
    int & itemId,
    std::vector<std::pair<int, float> >& itemVector) {

    assert(line.size() != 0);

    boost::char_separator<char> sep(" \t");
    boost::tokenizer<boost::char_separator<char>> tok(line, sep);

    bool setId = false;
    for(auto &w : tok) {
        if(!setId) {
            itemId = std::stoi(w);
            setId = true;
        } else itemVector.push_back( lshStofPair(w) );
    }
    assert (itemVector.size() != 0);
}

typedef SimhashQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashQuery;
typedef SimhashItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashItem;
typedef SimhashBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> FinalSimhashBucket;
