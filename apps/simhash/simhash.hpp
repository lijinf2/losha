#pragma once
#include <unordered_set>

#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "lshcore/lshbucket.hpp"
#include "lshcore/lshitem.hpp"
#include "lshcore/lshquery.hpp"
using namespace husky::losha;
using std::vector;
using std::pair;
using std::string;

template<
    typename ItemIdType,
    typename ItemElementType,
    typename QueryMsg,
    typename AnswerMsg
>
class SimhashQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit SimhashQuery(const typename SimhashQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}
    virtual void query( 
            LSHFactory<ItemIdType, ItemElementType>& factory,
            const vector<AnswerMsg>& inMsg) override {

        this->queryMsg = this->getItemId();
        for(auto &bk : factory.calItemBuckets(*this)) {
            this->sendToBucket(bk);
        }
    }
};

template<
    typename ItemIdType,
    typename ItemElementType,
    typename QueryMsg,
    typename AnswerMsg
>
class SimhashItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit SimhashItem(const typename SimhashItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}

    virtual void answer(
        LSHFactory<ItemIdType, ItemElementType>& factory,
        const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for(auto& queryId : inMsgs ) {
        	
        	if(evaluated.find( queryId ) != evaluated.end()) continue;
        	evaluated.insert(queryId );

        	auto& queryVector = 
                factory.getQueryVector(queryId);

        	float distance = factory.calDist(queryVector, this->getItemVector());

        	if( distance <= std::stof( husky::Context::get_param("radians") ) ) {
        		std::string result;
        		result += std::to_string( queryId ) + " ";
        		result += std::to_string( this->getItemId() ) + " " + std::to_string(distance) + "\n";
                husky::io::HDFS::Write( 
                    husky::Context::get_param("hdfs_namenode"),
                    husky::Context::get_param("hdfs_namenode_port"),
                    result,
                    husky::Context::get_param("outputPath"),
                    husky::Context::get_global_tid());
        	}
        }
    }
};

template<
    typename ItemIdType,
    typename ItemElementType,
    typename QueryMsg,
    typename AnswerMsg
>
class SimhashBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit SimhashBucket(const typename SimhashBucket::KeyT& bId):LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(bId){}
};
