#pragma once
#include "lshcore/lshbucket.hpp"
#include "lshcore/lshitem.hpp"
#include "lshcore/lshquery.hpp"
using namespace husky::losha;
using std::vector;
using std::pair;

//for sparse vector, maintain one copy for each process
typedef int ItemIdType;
typedef std::pair<int, float> ItemElementType;
typedef ItemIdType QueryMsg;
typedef pair<ItemIdType, float> AnswerMsg;

// for broadcasting
class PLSHQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit PLSHQuery(const typename PLSHQuery::KeyT& id):LSHQuery(id){}
    virtual void query( 
            LSHFactory<ItemIdType, ItemElementType>& factory,
            const vector<AnswerMsg>& inMsg) override {

        this->queryMsg = this->getItemId();
        this->broadcast();
        for(auto &bk : factory.calItemBuckets(*this)) {
            // get_worker().send_message( queryPoint, *it, bucket_list);
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

        std::set<ItemIdType> evaluated;

        // // std::bitset<1000> evaluated;
        // // for(auto& queryId : Husky::get_messages< ItemIdType >(*this) ) {
        // for(auto& queryId : getQueryMsg() ) {
        // 	//
        // 	if(evaluated.find( queryId ) != evaluated.end() ) continue;
        // 	evaluated.insert(queryId );
        //
        // 	// if(evaluated[queryId] == 1) continue;
        // 	// evaluated.set(queryId, 1) ;
        //
        // 	//broadcast will halt the string key
        // 	auto& queryVector = this->template get_response<
        // 		std::vector<ItemElementType> >(queryId);
        // 	float distance = factory.calDistance(queryVector, this->getItemVector());
        //
        // 	if(distance <= 0.9) {
        // 		std::string result;
        // 		result += std::to_string( queryId ) + " ";
        // 		result += std::to_string( this->getItemId() ) + " " + std::to_string(distance) + "\n";
        // 		Husky::HDFS::Write( Husky::Context::get_params("hdfs_namenode"), Husky::Context::get_params("hdfs_namenode_port"), result, LSHContext::getOutputPath(), get_worker().id );
        // 	}
        // }
    }
};

class PLSHBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit PLSHBucket(const typename PLSHBucket::KeyT& bId):LSHBucket(bId){}
};
