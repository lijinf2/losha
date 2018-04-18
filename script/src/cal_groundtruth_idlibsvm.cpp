#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>

#include "gqr/util/cal_groundtruth.h"
#include "lshcore/lshutils.cpp"
#include "losha/common/distor.hpp"
using namespace std;
using lshbox::GTQuery;

unsigned readIdLIBSVM(vector<vector<pair<int, float>>>& fvecs, ifstream& fin, unsigned maxNumRecords = UINT_MAX) {
    unsigned readNumRecords = 0;
    string line;

    while(readNumRecords < maxNumRecords && getline(fin, line)) {
        readNumRecords++;

        string token;
        istringstream iss(line);

        iss >> token; // the first one is item id

        vector<pair<int, float>> vec;
        while(iss >> token) {
            vec.emplace_back(husky::losha::lshStoPair(token));
        }
        fvecs.emplace_back(vec);
    }
    return readNumRecords;
}

// wrapper
float sparseCalAngularDist(
        const std::vector<std::pair<int, float>> & queryVector,
        const std::vector<std::pair<int, float>> & itemVector) {
    return husky::losha::calAngularDist(queryVector, itemVector, false);
}

int main(int argc, char** argv) {
    if (argc != 6 && argc != 7) {
        cout << "usage: program base_file.idlibsvm query_file.idlibsvm K groundtruth_file.lshbox groundtruth_file.ivecs num_threads=4" << endl;
        return 0;
    }

    const char* baseFileName = argv[1];
    const char* queryFileName = argv[2];
    int K = std::atoi(argv[3]);
    const char* lshboxBenchFileName = argv[4];
    string metric = argv[5];
    int numThreads = 4;
    if (argc >= 7)
        numThreads = stoi(argv[6]);

    int itemBatchSize = 200000;

    ifstream queryFin(queryFileName);
    if (!queryFin) {
        cout << "query File "  << queryFileName << " does not exist "<< endl;
        return 0;
    }

    typedef pair<int, float> FeatureType;
    vector<vector<FeatureType>> queryVecs;
    readIdLIBSVM(queryVecs, queryFin);
    queryFin.close();

    ifstream baseFin(baseFileName, ios::binary);
    if (!baseFin) {
        cout << "base File " << baseFileName << " does not exist" << endl;
        return 0;
    }

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

    int itemStartIdx = 0;
    vector<vector<FeatureType>> items;
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

    lshbox::GroundWriter writer;
    writer.writeLSHBOX(lshboxBenchFileName, queryObjs, K);
    return 0;
}
