#include "core/engine.hpp"
#include <functional>
#include <vector>
#include <utility>
using std::vector;
using std::pair;

namespace husky {
namespace losha {

class CCVertex {
public:
    int _vid;
    vector<int> _nbs;
    int _label; 
    explicit AdjRecord(int id) : _vid(id), _label(id) {
        _vid = id;
    }

};

class KNNConnectedComponents {
public:
    template<typename ObjT>
    static void run(
        husky::ObjList<ObjT>& obj_list,
        std::function<int(const ObjT&)> getVertexId, 
        std::function<vector<int>(const ObjT&)> getNeighbors);

    template<typename ObjT>
    static vector<pair<int, int>> KNNConnectedComponents::getClusteringDesc(
        husky::ObjList<ObjT>& obj_list,
        std::function<int(const ObjT&)> getLabel);

    template<typename ObjT>
    static void KNNConnectedComponents::reportClustering(
        husky::ObjList<ObjT>& vertex_list);
};

template<typename ObjT>
void KNNConnectedComponents::run(
    husky::ObjList<ObjT>& obj_list,
    std::function<int(const ObjT&)> getVertexId, 
    std::function<vector<int>(const ObjT&)> getNeighbors) {

    auto vertex_list =
        husky::ObjListStore::create_objlist<CCVertex>();
    
    thread_local auto& msgCh =
            husky::ChannelStore::create_push_channel<vector<int>>(obj_list, vertex_list);

    husky::list_execute(
        obj_list,
        {},
        {&msgCh},
        [&msgCh, &getVertexId, &getNeighbors](ObjT& obj){
        msgCh.push(getNeighbors(obj), getVertexId(obj);
    });

    thread_local auto& minCombineCh =
            husky::ChannelStore::create_push_combined_channel<int, husky::MinCombiner<int>>(vertex_list, vertex_list);
    husky::list_execute(
        vertex_list,
        {&msgCh},
        {&minCombineCh},
        [&msgCh, &minCombineCh](CCVertex& vertex){
        vertex._nbs = msgCh.get(vertex);
        for (auto nb : vertex._nbs) {
            if (nb > vertex._label) {
                minCombineCh.push(vertex._label, nb);
            }
        }
    });

    husky::lib::Aggregator<int> not_finished(0, [](int& a, const int& b) { a += b; });
    not_finished.to_reset_each_iter();
    not_finished.update(1);
    auto& agg_ch = husky::lib::AggregatorFactory::get_channel();
    husky::lib::AggregatorFactory::sync();
    int iteration = 0;


    while(not_finished.get_value()) {
        // calculate block with minimum items
        husky::list_execute(
            vertex_list,
            {&minCombineCh},
            {&minCombineCh, &agg_ch},
            [&minCombineCh, &not_finished](CCVertex& vertex){
            if (minCombineCh.hash_msg(vertex)) {
                int msg = minCombineCh.get(vertex); 
                if (vertex._label > msg) {
                    vertex._label = msg;
                    if (nb > vertex._label) {
                        minCombineCh.push(vertex._label, nb);
                    }
                    not_finished.update(1);
                }
            }
        });
        iteration++;
    }

    // report number of clusters 
    reportClustering(vertex_list);
}

template<typename ObjT>
vector<pair<int, int>> KNNConnectedComponents::getClusteringDesc(
    husky::ObjList<ObjT>& obj_list,
    std::function<int(const ObjT&)> getLabel) {

    vector<pair<int, int>> agg =
        keyValueCombine<ObjT, int, int, husky::SumCombiner<int>>(
            vertex_list,
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

template<typename ObjT>
void KNNConnectedComponents::reportClustering(
    husky::ObjList<ObjT>& vertex_list) {

    // report number of clusters 
    auto agg = KNNConnectedComponents::getClusteringDesc(vertex_list);
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "number of clusters is " << agg.size() << std::endl;
        husky::LOG_I << "present cluster_id and size in (cluster_id, size)" << std::endl;
        for (int i = 0; i < agg.size(); ++i) {
            husky::LOG_I << "(" << agg[i].first << ", " << agg[i].second << ")" << std::endl;
        }
    }
}

void knnconnected_components{
}
}
}
