#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include "gqr/util/cal_groundtruth.h"
using std::vector;
using std::pair;
using lshbox::GTQuery;

namespace husky {
namespace losha {

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

template<typename FeatureType>
class RadiusQuery : public GTQuery<FeatureType> {
public:
    float radius = 0;
    vector<pair<int, float>> results;

    RadiusQuery(
        const vector<FeatureType>& cont,
        float r,
        std::function<float(const vector<FeatureType>&, const vector<FeatureType>&)> functor)
        : GTQuery<FeatureType>(cont, 0, functor){
        radius = r;
    }

    void evaluate(const vector<FeatureType>& item, int itemId) override {
        float distance = this->distor(this->content, item);
        if (distance <= radius)
            results.emplace_back(make_pair(itemId, distance));
    }

    const std::vector<pair<int, float>>& getResults() const {
        return results;
    }

    const std::vector<pair<int, float>>& getAndSortResults() {
        sortResults();
        return results;
    }

    void sortResults() {
        std::sort(results.begin(), results.end(),
            [](const pair<int, float>& a, const pair<int, float>& b){
            if (a.second != b.second) {
                return a.second < b.second;
            } else {
                return a.first < b.first;
            }
        });
    }

    float getRadius() const {
        return radius;
    }

};

class QueryWriter{
public:
    template<typename FeatureType>
    void writeLSHBOX(const char* lshboxBenchFileName, const vector<RadiusQuery<FeatureType>>& queryObjs) {
        // lshbox file
        ofstream lshboxFout(lshboxBenchFileName);
        if (!lshboxFout) {
            cout << "cannot create output file " << lshboxBenchFileName << endl;
            assert(false);
        }
        float radius = queryObjs[0].getRadius();
        lshboxFout << queryObjs.size() << "\t" << radius << endl;

        for (int i = 0; i < queryObjs.size(); ++i) {
            assert(queryObjs[i].getRadius() == radius);
            lshboxFout << i << "\t";
            const vector<pair<int, float>>& pairs = queryObjs[i].getResults();
            for (int idx = 0; idx < pairs.size(); ++idx) {
                lshboxFout << pairs[idx].first << "\t" << pairs[idx].second << "\t";
            }
            lshboxFout << endl;
        }
        lshboxFout.close();
        cout << "lshbox groundtruth are written into " << lshboxBenchFileName << endl;
    }
};
}
}
