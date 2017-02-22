#pragma once
#include <unordered_set>
#include "lshcore/lshbucket.hpp"
#include "lshcore/lshitem.hpp"
#include "lshcore/lshquery.hpp"
using namespace husky::losha;
using std::vector;
using std::pair;

// For sparse vector
typedef int ItemIdType;
typedef std::pair<int, float> ItemElementType;
typedef ItemIdType QueryMsg;
typedef pair<ItemIdType, float> AnswerMsg;

// Broadcast queries
class PLSHQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit PLSHQuery(const typename PLSHQuery::KeyT& id):LSHQuery(id){}
    virtual void query( 
            LSHFactory<ItemIdType, ItemElementType>& factory,
            const vector<AnswerMsg>& inMsg) override {

        this->broadcast();
        this->queryMsg = this->getItemId();
        for(auto &bk : factory.calItemBuckets(*this)) {
            this->sendToBucket(bk);
        }
    }
};

// for broadcast
class PLSHItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit PLSHItem(const typename PLSHItem::KeyT& id):LSHItem(id){}

    virtual void answer(
        LSHFactory<ItemIdType, ItemElementType>& factory,
        const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for(auto& queryId : inMsgs ) {
        	
        	if(evaluated.find( queryId ) != evaluated.end() ) continue;
        	evaluated.insert(queryId );

        	auto& queryVector = 
                factory.getQueryVector(queryId);

            husky::LOG_I << "received query id " << queryId << std::endl;;
        	// float distance = factory.calDistance(queryVector, this->getItemVector());
            //
        	// if(distance <= 0.9) {
        	// 	std::string result;
        	// 	result += std::to_string( queryId ) + " ";
        	// 	result += std::to_string( this->getItemId() ) + " " + std::to_string(distance) + "\n";
        	// 	Husky::HDFS::Write( Husky::Context::get_params("hdfs_namenode"), Husky::Context::get_params("hdfs_namenode_port"), result, LSHContext::getOutputPath(), get_worker().id );
        	// }
        }
    }
};

class PLSHBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit PLSHBucket(const typename PLSHBucket::KeyT& bId):LSHBucket(bId){}
};
