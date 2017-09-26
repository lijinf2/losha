#include "e2lsh.hpp"
#include "boost/tokenizer.hpp"

// To parse small dataset (txt)
typedef int ItemIdType;
typedef int ItemElementType;
typedef ItemIdType QueryMsg;
typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
const int idBytes_ss = sizeof(ItemIdType);
const int dimension_ss = 5;
const int elementBytes_ss = sizeof(ItemElementType);

// input format: id f1 f2 f3 f4 f5
void setItemSMALL(
    boost::string_ref& line,
    int & itemId,
    std::vector<ItemElementType>& itemVector) {

    assert(line.size() != 0);

    boost::char_separator<char> sep(" \t");
    boost::tokenizer<boost::char_separator<char>> tok(line, sep);
    boost::tokenizer<boost::char_separator<char>>::iterator it = tok.begin();
    itemId = stoi(*it++);
    while (it != tok.end())
    {
        itemVector.push_back(stoi(*it++));
    }
    
    assert(itemVector.size() == dimension_ss);

}

typedef E2LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Query1B;
typedef E2LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Item1B;
typedef E2LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> Bucket1B;

