#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <cassert>
#include "../../gqr/apps/opq_evaluate.cpp"
using namespace std;
int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Usage: evalaute_triplets lshbox_file triplets_file" << endl;
        return -1;
    }

    const char* lshbox_file = argv[1];
    const char* triplets_file = argv[2];

    Bencher lshboxBench(lshbox_file);
    unsigned numQueries = lshboxBench.size();
    vector<vector<pair<unsigned, float>>> results(numQueries, vector<pair<unsigned, float>>());

    ifstream fin(triplets_file);

    if (!fin) {
        cout << "cannot open file " << triplets_file << endl;
        return -1;
    }

    string line;
    int queryId;
    int itemId;
    float distance;
    while(getline(fin, line)) {
        istringstream iss(line);
        iss >> queryId >> itemId >> distance;
        results[queryId].push_back(make_pair(itemId, distance));
    }

    for (auto& vec : results) {
        sort(vec.begin(), vec.end(),
            [](const pair<unsigned, float>& a, const pair<unsigned, float>& b) {
            return a.second < b.second;
        });
    }
    fin.close();

    cout << "avg recall:" << cal_avg_recall(lshboxBench, results, true) << endl;
    cout << "avg error: " << cal_avg_error(lshboxBench, results, true) << endl;

}
