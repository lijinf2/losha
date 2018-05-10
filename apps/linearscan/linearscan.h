#pragma once
#include <set>
#include <utility> 
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

#include "gqr/util/cal_groundtruth.h"
#include "losha/common/writer.hpp"
#include "lshcore/lshquery.hpp"
#include "lshcore/lshitem.hpp"
using namespace husky::losha;
using lshbox::TopK;

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class LSQuery : public LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit LSQuery(const typename LSQuery::KeyT& id):LSHQuery<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id) {}
    void query(LSHFactory<ItemIdType, ItemElementType>& fty, const vector<AnswerMsg>& inMsgs) override {
        if (inMsgs.size() != 0) {
            // sort
            TopK topK(std::stoi(husky::Context::get_param("topK")));
            topK.collect(inMsgs);

            // dump to HDFS
            writeHDFSPairVector(this->getItemId(), topK.getTopKPairs(), "hdfs_namenode", "hdfs_namenode_port", "outputPath");
        }
    }
};

template<typename ItemIdType, typename ItemElementType, typename QueryMsg, typename AnswerMsg>
class LSItem : public LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg> {
public:
    explicit LSItem(const typename LSItem::KeyT& id):LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>(id){}
    LSItem() : LSHItem<ItemIdType, ItemElementType, QueryMsg, AnswerMsg>() {}

    bool evaluated = false;
    virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory, const vector<QueryMsg>& inMsgs) {

        if (evaluated == true) 
            return;
        evaluated = true;
        const auto & queries = factory.getAllQueries();
        for (auto& p : queries) {

            const auto& queryId = p.first;
            const auto& queryVector = p.second;
            float distance = factory.calDist(queryVector, this->getItemVector());

            auto item_pair = std::make_pair(this->getItemId(), distance);
            this->sendToQueryTopk(
                queryId, 
                item_pair, 
                std::stoi(husky::Context::get_param("topK")),
                [](const AnswerMsg& a, const AnswerMsg& b){
                return a.second < b.second;
            });
        }
    }
};
