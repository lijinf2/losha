#pragma once
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "losha/common/writer.hpp"
#include "lshcore/lshbucket.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
using namespace husky::losha;

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class DefaultQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit DefaultQuery(const typename DefaultQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id) {}
    void query(LSHFactory<ItemIdType, ItemElementType>& fty, const vector<AnswerMsg>& inMsg) override {

        this->queryMsg = this->getItemId();
        for (auto& bId : fty.calItemBuckets(this->getQuery())) {
            this->sendToBucket(bId);
        }
    }
};

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class DefaultItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit DefaultItem(const typename DefaultItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}
    DefaultItem() : LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>() {}

    virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory, const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for (auto& queryId : inMsgs) {

            if (evaluated.find(queryId)!= evaluated.end()) continue;
            evaluated.insert(queryId);

            const auto& queryVector = factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());

            writeHDFSTriplet(queryId, this->getItemId(), distance, "hdfs_namenode", "hdfs_namenode_port", "outputPath");
        }
    }
};

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class DefaultBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit DefaultBucket(const typename DefaultBucket::KeyT& bId):LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(bId){}
};
