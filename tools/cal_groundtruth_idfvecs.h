#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include "gqr/util/cal_groundtruth.h"
#include "./cal_groundtruth_losha.h"
using std::vector;
using std::pair;
using namespace lshbox;

namespace husky {
namespace losha {

template<typename FeatureType>
unsigned readIdFVECS(vector<pair<int, vector<FeatureType>>>& fvecs, ifstream& fin, unsigned maxNumRecords = UINT_MAX) {
    unsigned readNumRecords = 0;
    int itemId;
    int dimension;
    while(readNumRecords < maxNumRecords && fin.read((char*)&itemId, sizeof(int))) {
        readNumRecords++;
        fin.read((char*)&dimension, sizeof(int));
        vector<FeatureType> vec;
        vec.resize(dimension);
        fin.read((char*)&vec[0], sizeof(FeatureType) * dimension);
        fvecs.emplace_back(std::make_pair(itemId, vec));
    }
    return readNumRecords;
}

template<typename FeatureType>
class TopkIdQuery : public GTQuery<FeatureType> {
public:
    int queryId;
    vector<pair<int, float>> results;

    TopkIdQuery(
        int qId, 
        const vector<FeatureType>& cont,
        int K,
        std::function<float(const vector<FeatureType>&, const vector<FeatureType>&)> functor)
        : GTQuery<FeatureType>(cont, K, functor){
        queryId = qId; 
    }

    int getId() const {
        return queryId;
    }

    static void writeLSHBOX(const char* lshboxBenchFileName, const vector<TopkIdQuery<FeatureType>>& queryObjs) {
        // lshbox file
        ofstream lshboxFout(lshboxBenchFileName);
        if (!lshboxFout) {
            cout << "cannot create output file " << lshboxBenchFileName << endl;
            assert(false);
        }
        int K = queryObjs[0].getK();
        lshboxFout << queryObjs.size() << "\t" << K << endl;
        for (int i = 0; i < queryObjs.size(); ++i) {
            assert(queryObjs[i].getK() == K);
            lshboxFout << queryObjs[i].getId() << "\t";
            auto topker = queryObjs[i].getTopK();
            for (int idx = 0; idx < topker.size(); ++idx) {
                lshboxFout << topker[idx].id << "\t" << topker[idx].distance << "\t";
            }
            lshboxFout << endl;
        }
        lshboxFout.close();
        cout << "lshbox groundtruth are written into " << lshboxBenchFileName << endl;
    }
};

template<typename QueryType, typename FeatureType>
void updateQueries(vector<QueryType*> queries, const vector<pair<int, vector<FeatureType>>>* itemsPtr) {
    for (auto& query: queries) {
        for (int i = 0; i < itemsPtr->size(); ++i) {
            query->evaluate((*itemsPtr)[i].second,(*itemsPtr)[i].first);
        }
    }
}

template<typename QueryType, typename FeatureType>
void updateAll(vector<QueryType>& queries, const vector<pair<int, vector<FeatureType>>>& items, int numThreads = 4) {
    vector<thread> threads;
    int numQueriesPerThread = queries.size() / numThreads + 1;

    int queryIdx = 0;
    vector<QueryType*> queryLinks;
    while(queryIdx < queries.size()) {
        queryLinks.push_back(&queries[queryIdx++]);
        if (queryLinks.size() == numQueriesPerThread) {
            threads.push_back(thread(updateQueries<QueryType, FeatureType>, queryLinks, &items));
            queryLinks.clear();
        }
    }
    threads.push_back(thread(updateQueries<QueryType, FeatureType>, queryLinks, &items));
    for (int i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
}

template<typename QueryType, typename FeatureType>
void evaluate(
    std::vector<QueryType>& queryObjs,
    const char* baseFileName,
    int numThreads = 4,
    int itemBatchSize = 200000) {

    ifstream baseFin(baseFileName, ios::binary);
    if (!baseFin) {
        cout << "base File " << baseFileName << " does not exist" << endl;
        assert(false);
    }
    int numItems = 0;
    vector<pair<int, vector<FeatureType>>> items;
    items.reserve(itemBatchSize);
    while (true) {
        readIdFVECS(items, baseFin, itemBatchSize);
        if (items.size() == 0) {
            break;
        }

        updateAll(queryObjs, items, numThreads);
        numItems += items.size();
        cout << numItems << " items have been evaluated" << endl;
        items.clear();
    }

    baseFin.close();
}


template<typename FeatureType>
vector<TopkIdQuery<FeatureType>> topk_evaluate(
    const vector<pair<int, vector<FeatureType>>>& queryVecs,
    const char* baseFileName,
    int K,
    string metric,
    int numThreads = 4,
    int itemBatchSize = 200000) {

    vector<TopkIdQuery<FeatureType>> queryObjs;
    for (int i = 0; i < queryVecs.size(); ++i) {
        if (metric == "euclidean") {
            queryObjs.push_back(TopkIdQuery<FeatureType>(queryVecs[i].first, queryVecs[i].second, K, calEuclideanDist));
        } else if (metric == "angular") {
            assert(false);
        } else if (metric == "product") {
            assert(false);
        } else {
            assert(false);
        }
    }
    evaluate<TopkIdQuery<FeatureType>, FeatureType>(queryObjs, baseFileName, numThreads);
    return queryObjs;
}


}
}
