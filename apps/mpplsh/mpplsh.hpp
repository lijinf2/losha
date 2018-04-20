#pragma once
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "losha/common/writer.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
#include "lshcore/densevector.hpp"
using namespace husky::losha;
using std::vector;
using std::pair;

template<typename ItemIdType, typename ItemElementType>
class MPPLSHQuery: public LSHQuery<ItemIdType, ItemElementType, ItemIdType, DenseVector<ItemIdType, ItemElementType>> {
public:
    unsigned iteration = 0;
    std::set<ItemIdType> evaluated;
    explicit MPPLSHQuery(
        const typename MPPLSHQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, ItemIdType, DenseVector<ItemIdType, ItemElementType>>(id) {}

    void query(
        LSHFactory<ItemIdType, ItemElementType>& fty,
        const vector<DenseVector<ItemIdType, ItemElementType>>& inMsg) override {

        if (iteration == 0) {
            this->queryMsg = this->getItemId();
            for (auto& bId : fty.calItemBuckets(this->getQuery())) {
                this->sendToBucket(bId);
            }
        } else {
            for (const auto& item : inMsg) {
                // check duplication
                ItemIdType itemId = item.getItemId();
                if (evaluated.find(itemId) != evaluated.end())
                    continue;
                evaluated.insert(itemId);

                // collect to HDFS
                float dist = fty.calDist(item.getItemVector(), this->getItemVector());
                writeHDFSTriplet(this->getItemId(), std::make_pair(itemId, dist), "hdfs_namenode", "hdfs_namenode_port", "outputPath");

                // issue new queries
                for (auto& bId : fty.calItemBuckets(item)) {
                    this->sendToBucket(bId);
                }
            }
        }
        iteration++;
    }
};

template<typename ItemIdType, typename ItemElementType>
class MPPLSHItem: public LSHItem<ItemIdType, ItemElementType, ItemIdType, DenseVector<ItemIdType, ItemElementType>> {
public:
    explicit MPPLSHItem(const typename MPPLSHItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, ItemIdType, DenseVector<ItemIdType, ItemElementType>>(id){}

    virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory, const vector<ItemIdType>& inMsgs) override {

        for (const auto& queryId : inMsgs)
        {
            const auto& queryVector = factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());
            if (distance <= 0.9) {
                this->sendToQuery(queryId, this->getItem());
            }
        }
    }
};
