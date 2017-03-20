#include "lshcore/util/topkpairs.hpp"
#include <iostream>
#include <utility>
#include <vector>
using namespace std;
int main() {
    vector<pair<int, float>> vec;
    for(int i = 0; i < 5; ++i){
        vec.push_back(std::pair<int, float>(i, 5.0 - i));
    }

    husky::losha::TopkPairs<int, float> topk(3);
    for (int i = 0; i < vec.size(); ++i) {
        topk.push(vec[i]);
    }

    vector<pair<int, float>> result = topk.popAll();
    for (int i = 0; i < result.size(); ++i) {
        cout << result[i].first << " " << result[i].second << endl;
    }
    for (int i = 0; i < result.size(); ++i) {
        assert(result[i].first = 4 - i);
        assert(result[i].second = vec[4 - i].second);
    }
}
