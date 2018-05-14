#include <functional>
#include <vector>
#include <utility>

#include "core/engine.hpp"
#include "lib/aggregator_factory.hpp"

#include "knnagg.h"
using std::vector;
using std::pair;

namespace husky {
namespace losha {

class KNNConnectedComponents {
public:
    template<typename ObjT>
    static void compute(
        husky::ObjList<ObjT>& obj_list,
        std::function<int& (ObjT&)> getLabelRef, 
        std::function<const vector<int>& (ObjT&)> getNeighborsRef);

    template<typename ObjT>
    static vector<pair<int, int>> getClusteringDesc(
        husky::ObjList<ObjT>& obj_list,
        std::function<int(const ObjT&)> getLabel);

    static void reportClustering(
        const vector<pair<int, int>>& blockToSize);
};

template<typename ObjT>
void KNNConnectedComponents::compute(
    husky::ObjList<ObjT>& obj_list,
    std::function<int& (ObjT&)> getLabelRef, 
    std::function<const vector<int>& (ObjT&)> getNeighborsRef) {

        int numNodes = count(obj_list);
    thread_local auto& minCombineCh =
            husky::ChannelStore::create_push_combined_channel<int, husky::MinCombiner<int>>(obj_list, obj_list);

    husky::list_execute(
        obj_list,
        {},
        {&minCombineCh},
        [&minCombineCh, &getLabelRef, &getNeighborsRef](ObjT& obj){
        getLabelRef(obj) = obj.id();
        for (const int& nb : getNeighborsRef(obj)) {
            if (nb > getLabelRef(obj)) {
                minCombineCh.push(getLabelRef(obj), nb);
            }
        }
    });

    husky::lib::Aggregator<int> not_finished(0, [](int& a, const int& b) { a += b; });
    not_finished.to_reset_each_iter();
    auto& agg_ch = husky::lib::AggregatorFactory::get_channel();
    husky::lib::AggregatorFactory::sync();
    int iteration = 0;

    while(true) {
        husky::list_execute(
            obj_list,
            {&minCombineCh},
            {&minCombineCh, &agg_ch},
            [&minCombineCh, &not_finished, &getLabelRef, &getNeighborsRef](ObjT& obj){
            if (minCombineCh.has_msgs(obj)) {
                int msg = minCombineCh.get(obj); 
                if (getLabelRef(obj) > msg) {
                    not_finished.update(1);
                    getLabelRef(obj) = msg;
                    for (auto nb : getNeighborsRef(obj)) {
                        if (nb > msg) {
                            minCombineCh.push(msg, nb);
                        }
                    }
                }
            }
        });

        int numActive = not_finished.get_value();
        if (numActive == 0)
            break;
        iteration++;
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "finish iteration " << iteration << std::endl;
        }
    }
}

template<typename ObjT>
vector<pair<int, int>> KNNConnectedComponents::getClusteringDesc(
    husky::ObjList<ObjT>& obj_list,
    std::function<int(const ObjT&)> getLabel) {

    vector<pair<int, int>> agg =
        keyValueCombine<ObjT, int, int, husky::SumCombiner<int>>(
            obj_list,
            [&getLabel](const ObjT& v){ return getLabel(v);},
            [](const ObjT& v){ return 1;});

    std::sort(
        agg.begin(),
        agg.end(),
        [](const pair<int, int>& a, const pair<int, int>&b){
            return a.second > b.second;
        });

    return agg;
}

void KNNConnectedComponents::reportClustering(
    const vector<pair<int, int>>& blockToSize) {

    // report number of clusters 
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "number of clusters is " << blockToSize.size() << std::endl;
        husky::LOG_I << "present cluster_id and size in (cluster_id, size)" << std::endl;
        for (int i = 0; i < blockToSize.size(); ++i) {
            husky::LOG_I << "(" << blockToSize[i].first << ", " << blockToSize[i].second << ")" << std::endl;
        }
    }
}

}
}
