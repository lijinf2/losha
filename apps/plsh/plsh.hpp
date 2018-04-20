#pragma once
#include <unordered_set>

#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "lshcore/lshitem.hpp"
using namespace husky::losha;
using std::vector;

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class PLSHItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> 
{
public:
    explicit PLSHItem(const typename PLSHItem::KeyT& id) : LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}

    virtual void answer(
        LSHFactory<ItemIdType, ItemElementType>& factory,
        const vector<QueryMsg>& inMsgs) {

        std::unordered_set<QueryMsg> evaluated;
        for (auto& queryId : inMsgs) {

            if (evaluated.find(queryId)!= evaluated.end()) continue;
            evaluated.insert(queryId);

            const auto& queryVector = factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());

            if (distance <= 0.9)
                writeHDFSTriplet(queryId, this->getItemId(), distance, "hdfs_namenode", "hdfs_namenode_port", "outputPath");
        }
    }
};
