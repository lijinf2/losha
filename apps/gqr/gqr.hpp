#pragma once
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <mutex>
#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "gqr/include/lshbox/query/tree.h"
#include "gqr/include/lshbox/query/tstable.h"
#include "gqr/util/cal_groundtruth.h"

#include "losha/common/writer.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
using namespace husky::losha;
using lshbox::TopK;
using lshbox::IdAndDstPair;

Tree* GLOBAL_Tree = NULL;
std::once_flag fvs_flag;

template<
    typename ItemIdType,
    typename ItemElementType,
    typename QueryMsg,
    typename AnswerMsg>
class GQRQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    unsigned iteration = 0;
    TopK topk;
    explicit GQRQuery(
        const typename GQRQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id)
        , topk(20) {}
    void query(LSHFactory<ItemIdType, ItemElementType>& fty, const vector<AnswerMsg>& inMsg) override {

        // initliaze fvs
        std::call_once(fvs_flag, [&fty]() {
                GLOBAL_Tree = new Tree(fty.getRow());
        });

        if (iteration == 0) {
            initialize(fty, GLOBAL_Tree);
            this->queryMsg = this->getItemId();
            for (auto& bId : fty.calItemBuckets(this->getQuery())) {
                this->sendToBucket(bId);
            }
        }  else {
            topk.collect(inMsg);

            if (iteration == 20) {
                auto result = topk.getTopK();
                for (const auto& e : result)
                    writeHDFSTriplet(this->getItemId(), std::make_pair(e.id, e.distance), "hdfs_namenode", "hdfs_namenode_port", "outputPath");
                // this function will not be invoked once set finished
                this->setFinished();
                return;
            }

            vector<int> sig(1);
            for (int tb = 0; tb < handlers_.size(); ++tb) {
                if (handlers_[tb].moveForward()) {
                    unsigned long long tmp = handlers_[tb].getCurBucket();
                    sig[0] = (int)tmp;
                    this->sendToBucket(sig, tb);
                }
            }
        }
        iteration++;
    }

private:
    std::vector<TSTable> handlers_;
    void initialize(const LSHFactory<ItemIdType, ItemElementType>& fty, Tree* tree) {
        int numTables = fty.getBand();
        handlers_.reserve(numTables);

        auto projs = fty.calProjs(this->getQuery());
        assert(projs.size() == numTables);
        for (unsigned t = 0; t < numTables; ++t) {

            vector<bool> hashBits(projs[t].size());
            for (int idx = 0; idx < hashBits.size(); ++idx) {
                if (projs[t][idx] >= 0 ) {
                    hashBits[idx] = 1;
                } else {
                    hashBits[idx] = 0;
                }
                projs[t][idx] = fabs(projs[t][idx]);
            }

            handlers_.emplace_back(TSTable(hashBits, projs[t], tree));
        }
    }
};

template<
    typename ItemIdType,
    typename ItemElementType, 
    typename QueryMsg, 
    typename AnswerMsg
>
class GQRItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit GQRItem(const typename GQRItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}

    void answer (
            LSHFactory<ItemIdType, ItemElementType>& factory,
            const vector<QueryMsg>& inMsgs) override {

        std::unordered_set<QueryMsg> evaluated;
        for (const auto& queryId : inMsgs)
        {
            if (evaluated.find(queryId)!= evaluated.end()) continue;
            evaluated.insert(queryId);

            const auto& queryVector = factory.getQueryVector(queryId);
            float distance = factory.calDist(queryVector, this->getItemVector());
            std::string result;
            result += std::to_string(queryId) + " ";
            result += std::to_string(this->getItemId()) + " " + std::to_string(distance) + "\n";

            auto item_pair = std::make_pair(this->getItemId(), distance);
            this->sendToQuery(queryId, item_pair);
        }
    }
};
