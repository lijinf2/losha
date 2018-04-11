#pragma once
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "lshcore/lshbucket.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
using namespace husky::losha;

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class E2LSHQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit E2LSHQuery(const typename E2LSHQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id) {}
    void query(LSHFactory<ItemIdType, ItemElementType>& fty, const vector<AnswerMsg>& inMsg) override {

        this->queryMsg = this->getItemId();
        for (auto& bId : fty.calItemBuckets(this->getQuery())) {
            this->sendToBucket(bId);
        }
    }
};

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class E2LSHItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit E2LSHItem(const typename E2LSHItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}
    E2LSHItem() : LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>() {}

    virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory, const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for (auto& queryId : inMsgs) {

            if (evaluated.find(queryId)!= evaluated.end()) continue;
            evaluated.insert(queryId);

            auto& queryVector = factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());

            std::string result = std::to_string(queryId) + " ";
            result += std::to_string(this->getItemId()) + " " + std::to_string(distance) + "\n";
            
            husky::io::HDFS::Write(
                husky::Context::get_param("hdfs_namenode"), husky::Context::get_param("hdfs_namenode_port"),
                result,
                husky::Context::get_param("outputPath"), husky::Context::get_global_tid());
        }
    }
};

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class E2LSHBucket: public LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit E2LSHBucket(const typename E2LSHBucket::KeyT& bId):LSHBucket<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(bId){}
};
