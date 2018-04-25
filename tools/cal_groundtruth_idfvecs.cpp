#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>

#include "gqr/util/cal_groundtruth.h"
#include "lshcore/lshutils.cpp"
#include "losha/common/distor.hpp"

#include "./cal_groundtruth_idfvecs.h"
using namespace std;
using lshbox::GTQuery;
using namespace husky::losha;

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

    typedef float FeatureType;
    vector<pair<int, vector<FeatureType>>> queryVecs;
    readIdFVECS(queryVecs, queryFin);
    queryFin.close();

    if (queryType.find("topk") != string::npos) {
        int K = stoi(queryType.substr(5));
        vector<TopkIdQuery<FeatureType>> queryObjs = topk_evaluate(queryVecs, baseFileName, K, metric, numThreads);
        lshbox::GroundWriter writer;
        TopkIdQuery<FeatureType>::writeLSHBOX(lshboxBenchFileName, queryObjs);
    } else {
        std::cout << "you must provide query type such as: topk:20 or radius:0.9" << std::endl;
        assert(false);
    }

    return 0;
}
