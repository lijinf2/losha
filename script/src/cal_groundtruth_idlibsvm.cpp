#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>

#include "gqr/util/cal_groundtruth.h"
#include "lshcore/lshutils.cpp"
#include "losha/common/distor.hpp"

#include "./cal_groundtruth_idlibsvm.h"
using namespace std;
using lshbox::GTQuery;
using namespace husky::losha;

template<typename QueryType>
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
    int itemStartIdx = 0;
    vector<vector<pair<int, float>>> items;
    items.reserve(itemBatchSize);
    while (true) {
        readIdLIBSVM(items, baseFin, itemBatchSize);
        if (items.size() == 0) {
            break;
        }

        updateAll(queryObjs, items, itemStartIdx, numThreads);
        itemStartIdx += items.size();
        cout << itemStartIdx << " items have been evaluated" << endl;
        items.clear();
    }

    baseFin.close();
}

template<typename FeatureType>
vector<GTQuery<FeatureType>> topk_evaluate(
    const std::vector<std::vector<FeatureType>>& queryVecs,
    const char* baseFileName,
    int K,
    string metric,
    int numThreads = 4,
    int itemBatchSize = 200000) {

    vector<GTQuery<FeatureType>> queryObjs;
    for (int i = 0; i < queryVecs.size(); ++i) {
        if (metric == "euclidean") {
            assert(false);
        } else if (metric == "angular") {
            queryObjs.push_back(GTQuery<FeatureType>(queryVecs[i], K, sparseCalAngularDist));
        } else if (metric == "product") {
            assert(false);
        } else {
            assert(false);
        }
    }
    evaluate(queryObjs, baseFileName, numThreads);
    return queryObjs;
}

template<typename FeatureType>
vector<RadiusQuery<FeatureType>> radius_evaluate(
    const std::vector<std::vector<FeatureType>>& queryVecs,
    const char* baseFileName,
    float radius,
    string metric,
    int numThreads = 4,
    int itemBatchSize = 200000) {

    vector<RadiusQuery<FeatureType>> queryObjs;
    for (int i = 0; i < queryVecs.size(); ++i) {
        if (metric == "euclidean") {
            assert(false);
        } else if (metric == "angular") {
            queryObjs.push_back(RadiusQuery<FeatureType>(queryVecs[i], radius, sparseCalAngularDist));
        } else if (metric == "product") {
            assert(false);
        } else {
            assert(false);
        }
    }
    evaluate(queryObjs, baseFileName, numThreads);
    for (auto& query : queryObjs) {
        query.sortResults();
    }
    return queryObjs;
}

int main(int argc, char** argv) {
    if (argc != 6 && argc != 7) {
        cout << "usage: program base_file.idlibsvm query_file.idlibsvm K groundtruth_file.lshbox groundtruth_file.ivecs num_threads=4" << endl;
        return 0;
    }

    const char* baseFileName = argv[1];
    const char* queryFileName = argv[2];
    string queryType = argv[3];
    const char* lshboxBenchFileName = argv[4];
    string metric = argv[5];
    int numThreads = 4;
    if (argc >= 7)
        numThreads = stoi(argv[6]);


    ifstream queryFin(queryFileName);
    if (!queryFin) {
        cout << "query File "  << queryFileName << " does not exist "<< endl;
        return 0;
    }

    typedef pair<int, float> FeatureType;
    vector<vector<FeatureType>> queryVecs;
    readIdLIBSVM(queryVecs, queryFin);
    queryFin.close();

    if (queryType.find("topk") != string::npos) {
        int K = stoi(queryType.substr(5));
        vector<GTQuery<FeatureType>> queryObjs = topk_evaluate(queryVecs, baseFileName, K, metric, numThreads);
        lshbox::GroundWriter writer;
        writer.writeLSHBOX(lshboxBenchFileName, queryObjs);
    } else if (queryType.find("radius") != string::npos) {
        float radius = stof(queryType.substr(7));
        vector<RadiusQuery<FeatureType>> queryObjs = radius_evaluate(queryVecs, baseFileName, radius, metric, numThreads);
        QueryWriter writer;
        writer.writeLSHBOX(lshboxBenchFileName, queryObjs);
    } else {
        std::cout << "you must provide query type such as: topk:20 or radius:0.9" << std::endl;
        assert(false);
    }

    return 0;
}
