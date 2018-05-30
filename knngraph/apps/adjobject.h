#pragma once
#include <vector>
#include <utility>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <limits>

#include "gqr/util/cal_groundtruth.h"
#include "losha/common/aggre.hpp"
#include "losha/common/timer.hpp"

#include "dataobject.h"
#include "dataagg.h"
using lshbox::TopK;
using std::vector;
using std::pair;
using std::unordered_set;

namespace husky {
namespace losha {

class AdjObject{
public:
    using KeyT = int;
    KeyT _vid;
    // only sampled objects in lshbox file will have _trueKNN, other points have no _true_KNN;
    vector<pair<int, float>> _trueKNN;

    // should maintain a heap structure
    vector<pair<int, float>> _foundKNN;

    vector<int> _rKNN;
    int _label;
    unsigned char _state = 0;

    explicit AdjObject(const KeyT& id) : _vid(id), _label(id) {};
    const KeyT id() const {return _vid;};


    void setTrueKNN(const vector<pair<int, float>>& trueKNN) {
        _trueKNN = trueKNN;
    }

    void mergeFoundKNN(const vector<pair<int, float>>& msgs) {
        unordered_set<int> visited;
        visited.reserve(msgs.size() + _foundKNN.size());

        float maxLimitDist = std::numeric_limits<float>::max();
        for (const auto& p : _foundKNN) {
            if (p.second != maxLimitDist) {
                visited.insert(p.first);
            }
        }

        auto comparator = 
            [](const pair<int, float>& a, const pair<int, float>& b){
                return a.second < b.second;
        };

        for (const auto& p : msgs) {
            if (visited.find(p.first) != visited.end()) continue;
            visited.insert(p.first);

            if (p.second < _foundKNN.front().second) {
                std::pop_heap(_foundKNN.begin(), _foundKNN.end(), comparator);
                _foundKNN.pop_back();
                _foundKNN.emplace_back(p);
                std::push_heap(_foundKNN.begin(), _foundKNN.end(), comparator);
            }
        }

        // TopK topk(_foundKNN.size());
        // for (const auto& p : _foundKNN) {
        //     if (visited.find(p.first) != visited.end()) continue;
        //     visited.insert(p.first);
        //     topk.insert(p);
        // }
        // for (const auto& p : msgs) {
        //     if (visited.find(p.first) != visited.end()) continue;
        //     visited.insert(p.first);
        //     topk.insert(p);
        // }
        // _foundKNN = topk.getTopKPairs();
    }

    vector<int> getNB() {
        vector<int> nbs;
        for (const auto& p : _foundKNN) {
            nbs.push_back(p.first);
        }
        for (auto rNB : _rKNN) {
            nbs.push_back(rNB);
        }
        return nbs;
    }

    static void initFromLSHBOX(
        const string& lshboxPath, 
        husky::ObjList<AdjObject>& adj_list,
        int maxItemId,
        int numNBPerNode) {
        loadSampleGroundtruth(lshboxPath, adj_list);
        husky::list_execute(
            adj_list,
            [&maxItemId, &numNBPerNode](AdjObject& adj) {
                adj.randomInitFoundKNN(maxItemId, numNBPerNode);

            });
        updateRKNN(adj_list);
    }

    static void updateRKNN(
        husky::ObjList<AdjObject>& adj_list);

    static void bfsClustering(
        husky::ObjList<AdjObject>& adj_list, 
        unordered_set<unsigned> labels);

    static void randomClustering(
        husky::ObjList<AdjObject>& adj_list, 
        unordered_set<unsigned> labels);

    // statistics, (vid, max, vid, min)
    static vector<int> getRKNNSizeMaxMin(
        husky::ObjList<AdjObject>& adj_list);

    static float calSampleAvgRecall(
        husky::ObjList<AdjObject>& adj_list);

    static void resetLabels(
        husky::ObjList<AdjObject>& adj_list);

    static husky::PushChannel<int, AdjObject>& getToAdjPushIntCH(
        husky::ObjList<AdjObject>& adj_list) {
        if (_Adj2AdjPushIntCH == NULL) {
            _Adj2AdjPushIntCH = 
                &(husky::ChannelStore::create_push_channel<int>(adj_list, adj_list));
        }
        return *(_Adj2AdjPushIntCH);
    };

    static unordered_set<unsigned> getKIdMaxIndegree(
        husky::ObjList<AdjObject>& adj_list,
        int numBlocks);

    template<typename ItemElementType>
    static void trainFitMem(
        husky::ObjList<AdjObject>& adj_list,
        DataAgg<ItemElementType>& dataset,
        std::function<float (const vector<ItemElementType>& a, const vector<ItemElementType>& b)> distor);
private:
    static thread_local husky::PushChannel<int, AdjObject>* _Adj2AdjPushIntCH;
    float getCurRecall() {
        assert (_trueKNN.size() != 0);

        unordered_set<int> trueIvecs;
        for (auto p : _trueKNN) {
            trueIvecs.insert(p.first);
        }
        int matched = 0;
        for (auto p : _foundKNN) {
            if (trueIvecs.find(p.first) != trueIvecs.end()) {
                matched++;
            }
        }
        float recall = (float) matched / (float) std::min(_foundKNN.size(), _trueKNN.size());
        return recall;
    }

    static vector<pair<int, int>> getClusteringDesc(
        husky::ObjList<AdjObject>& adj_list) {

        vector<pair<int, int>> agg =
            keyValueCombine<AdjObject, int, int, husky::SumCombiner<int>>(
                adj_list,
                [](const AdjObject& v){ return v._label;},
                [](const AdjObject& v){ return 1;});

        std::sort(
            agg.begin(),
            agg.end(),
            [](const pair<int, int>& a, const pair<int, int>&b){
                return a.second > b.second;
            });

        return agg;
    }

    static void reportClustering(
        husky::ObjList<AdjObject>& adj_list) {

        // report number of clusters 
        auto agg = getClusteringDesc(adj_list);
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "number of clusters is " << agg.size() << std::endl;
            husky::LOG_I << "present cluster_id and size in (cluster_id, size)" << std::endl;
            for (int i = 0; i < agg.size(); ++i) {
                husky::LOG_I << "(" << agg[i].first << ", " << agg[i].second << ")" << std::endl;
            }
        }
    }

    template<typename ChannelType>
    void propagateLabel(ChannelType& ch) {
        for (const auto& p : _foundKNN) {
             ch.push(_label, p.first);
        }
        for (auto rNB : _rKNN) {
            ch.push(_label, rNB);
        }
    }
    static void loadSampleGroundtruth(
        const string& inputPath, 
        husky::ObjList<AdjObject>& obj_list);

    void randomInitFoundKNN(
        int maxItemId,
        int numNBPerNode) {

        std::default_random_engine generator(_vid);
        std::uniform_int_distribution<unsigned> distribution(0, maxItemId - 1);

        std::unordered_set<unsigned> inserted;
        float maxDist = std::numeric_limits<float>::max();

        while(_foundKNN.size() < numNBPerNode) {
            unsigned genId = distribution(generator);
            if (genId != _vid
                && inserted.find(genId) == inserted.end()) {
                inserted.insert(genId);
                _foundKNN.emplace_back(std::make_pair(genId, maxDist));
            }
        }

    }
};

void AdjObject::loadSampleGroundtruth(
    const string& inputPath, 
    husky::ObjList<AdjObject>& obj_list) {

    auto& lineInputformat =
        husky::io::InputFormatStore::create_line_inputformat();
    lineInputformat.set_input(inputPath);

    auto& channel = 
        husky::ChannelStore::create_push_channel<
           vector<pair<int, float>>>(lineInputformat, obj_list);

    auto parser = [&channel](boost::string_ref& line) {
        boost::char_separator<char> sep(" \t");
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        int size = 0;
        for (auto& w : tok) {
            size++;
        }
        if (size <= 2) return;

        int queryId;
        int itemId;
        float distance;
        vector<pair<int, float>> knn;

        auto it = tok.begin();
        queryId = std::stoi(*it++);
        // abandon the first pair, in which itemId is queryId, and the distance is 0;
        it++;
        it++;
        while(it != tok.end()) {
            itemId = std::stoi(*it++);
            distance = std::stof(*it++);
            knn.emplace_back(std::make_pair(itemId, distance));
        }
        channel.push(knn, queryId);
    };
    husky::load(lineInputformat, parser);

    husky::list_execute(obj_list,
        [&channel](AdjObject& obj){
        const auto msgs = channel.get(obj);
        if (msgs.size() == 0)
            return;
        else
            assert(msgs.size() == 1);
        obj.setTrueKNN(msgs[0]);
    });
}

void AdjObject::updateRKNN(
    husky::ObjList<AdjObject>& adj_list) {
     
    // auto& channel = AdjObject::getToAdjPushIntCH(adj_list);
    thread_local auto& channel = husky::ChannelStore::create_push_channel<int>(adj_list, adj_list);

    husky::list_execute(
        adj_list,
        {},
        {&channel},
        [&channel](AdjObject& adj){
            for(const auto& p : adj._foundKNN) {
                channel.push(adj.id(), p.first);
            }
        });

    husky::list_execute(
        adj_list,
        {&channel},
        {},
        [&channel](AdjObject& adj){
            adj._rKNN = channel.get(adj);
        });
}

void AdjObject::bfsClustering(
    husky::ObjList<AdjObject>& adj_list, 
    unordered_set<unsigned> labels) {

    AdjObject::resetLabels(adj_list);
    // cal cc until there are approximate cc blocks 
    
    thread_local auto& msgCh =
            husky::ChannelStore::create_push_channel<int>(adj_list, adj_list);
    husky::list_execute(
        adj_list,
        {},
        {&msgCh},
        [&msgCh, &labels](AdjObject& adj){
            if (labels.find(adj.id()) != labels.end() ) {
                adj._state = 1;
                adj.propagateLabel(msgCh);
            }
    });


    husky::lib::Aggregator<int> not_finished(0, [](int& a, const int& b) { a += b; });
    not_finished.to_reset_each_iter();
    not_finished.update(1);
    auto& agg_ch = husky::lib::AggregatorFactory::get_channel();
    husky::lib::AggregatorFactory::sync();
    int iteration = 0;
    while(true) {
        // calculate block with minimum items
        husky::list_execute(
            adj_list,
            {&msgCh},
            {&msgCh, &agg_ch},
            [&msgCh, &not_finished, &labels](AdjObject& adj){
            if (msgCh.get(adj).size() != 0 && adj._state == 0) {
                adj._state = 1;

                // // // assign to the k-th largest number
                // vector<int> msgs = msgCh.get(adj); 
                // int k = adj.id() % msgs.size();
                // std::nth_element(msgs.begin(), msgs.begin() + k, msgs.end());
                // std::sort(msgs.begin(), msgs.end());
                // adj._label = msgs[k];

                //
                // assign to the most frequent labels
                const vector<int>& msgs = msgCh.get(adj); 
                unordered_map<int, int> m;
                m.reserve(labels.size());
                pair<int, int> selected(-1, std::numeric_limits<int>::min());
                for (auto label : msgs) {
                    if (m.find(label) != m.end()) {
                        m[label]++;        
                    } else {
                        m[label] = 1;
                    }
                    if (m[label] > selected.second) {
                        selected.first = label;
                        selected.second = m[label];
                    }
                }
                adj._label = selected.first;

                adj.propagateLabel(msgCh);
                not_finished.update(1);
            }
        });
        int numNewAssigned = not_finished.get_value();
        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << "BFS Clustering finished round " << iteration << " finished with " << numNewAssigned << " items assigned " << std::endl;
        }

        if (numNewAssigned == 0)
            break;

        iteration++;
    }

    // get number of points that are not categorized
    int numUnAssigned = 
        sumAgg<AdjObject, int>(adj_list, [](AdjObject& adj){
                if (adj._state == 0)
                    return 1;
                else 
                    return 0;
    });

    if (numUnAssigned != 0) {
        // assigned unassinged vertices to the block with largest id
        unsigned maxLabel = std::numeric_limits<unsigned>::min();
        for (auto label : labels) {

            if (label > maxLabel) {
                maxLabel = label;
            }

        }

        husky::list_execute(
            adj_list,
            {},
            {},
            [&maxLabel](AdjObject& adj){
            if (adj._state == 0) {
                adj._label = maxLabel;
                adj._state = 1;
            }
        });

        if (husky::Context::get_global_tid() == 0) {
            husky::LOG_I << numUnAssigned << " unassigned records are assigned to block " << maxLabel << std::endl;
        }
    }
    
    // report number of clusters 
    // reportClustering(adj_list);
}

void AdjObject::randomClustering(
    husky::ObjList<AdjObject>& adj_list, 
    unordered_set<unsigned> labels) {

    AdjObject::resetLabels(adj_list);
    vector<int> labelsVec;
    labelsVec.reserve(labels.size());
    for (auto e : labels) {
        labelsVec.push_back(e);
    }
    
    // auto& msgCh =
    //         husky::ChannelStore::create_push_channel<int>(adj_list, adj_list);

    auto& msgCh = AdjObject::getToAdjPushIntCH(adj_list);
    husky::list_execute(
        adj_list,
        {},
        {&msgCh},
        [&msgCh, &labelsVec](AdjObject& adj){
            adj._label = labelsVec[adj.id() % labelsVec.size()];
    });

    // reportClustering(adj_list);

}

vector<int> AdjObject::getRKNNSizeMaxMin(
    husky::ObjList<AdjObject>& adj_list) {

    pair<int, unsigned> id_max =
        keyValueAggMax<AdjObject, int, unsigned>(
        adj_list, 
        [] (const AdjObject& obj) {
            return std::make_pair(obj.id(), obj._rKNN.size());});

    pair<int, unsigned> id_min =
        keyValueAggMin<AdjObject, int, unsigned>(
        adj_list, 
        [] (const AdjObject& obj) {
            auto p = std::make_pair(obj.id(), obj._rKNN.size());
            return p;}
        );

    vector<int> results;
    results.push_back(id_max.first);
    results.push_back(id_max.second);
    results.push_back(id_min.first);
    results.push_back(id_min.second);
    return results;
}

float AdjObject::calSampleAvgRecall(
    husky::ObjList<AdjObject>& adj_list) {

    int numSamples = 
        sumAgg<AdjObject, int> (
            adj_list,
            [](AdjObject& adj){
                int loadedSize = adj._trueKNN.size();
                if (loadedSize > 0)
                    return 1;
                else 
                    return 0;
            });

    float sumRecall = 
        sumAgg<AdjObject, float> (
            adj_list,
            [](AdjObject& adj){
                float recall = 0;
                if (adj._trueKNN.size() != 0) {
                    recall = adj.getCurRecall();
                }
                return recall;
        });

    float avg_recall = sumRecall / numSamples;
    return avg_recall;
}

void AdjObject::resetLabels(
    husky::ObjList<AdjObject>& adj_list) {
    husky::list_execute(
        adj_list,
        [](AdjObject& adj){
            adj._state = 0;
            adj._label = adj.id();
        });
}

unordered_set<unsigned> AdjObject::getKIdMaxIndegree(
    husky::ObjList<AdjObject>& adj_list,
    int numBlocks) {
	vector<pair<unsigned, int>> result =
        keyValueAggTopK<AdjObject, unsigned, int>(
        adj_list, 
        [] (const AdjObject& obj) {
            auto p = std::make_pair(obj.id(), obj._rKNN.size());
            return p;},
        numBlocks
        );

    unordered_set<unsigned> topk_set;
    for(auto& ele : result) {
        topk_set.insert(ele.first);
    }
    return topk_set;
}

template<typename ItemElementType>
void AdjObject::trainFitMem(
    husky::ObjList<AdjObject>& adj_list,
    DataAgg<ItemElementType>& dataset,
    std::function<float (const vector<ItemElementType>& a, const vector<ItemElementType>& b)> distor) {

    AccTimer timer;

    //@ avoid three copies of the dataset, probably write disk and then read disk
    thread_local auto& result_channel = 
        husky::ChannelStore::create_push_channel<pair<int, float>>(adj_list, adj_list);
    husky::list_execute(
        adj_list,
        {},
        {&result_channel},
        [&result_channel, &distor, &dataset](AdjObject& adj){
            // all pair training inside each record
            vector<int> record;
            record.reserve(adj._foundKNN.size() + adj._rKNN.size());
            for (auto& p : adj._foundKNN) {
                record.push_back(p.first);
            }
            for (auto id : adj._rKNN) {
                record.push_back(id);
            }

            for (int i = 0; i < record.size(); ++i) {
                int leftId = record[i];

                for (int j = i + 1; j < record.size(); ++j) {
                    int rightId = record[j];

                    const vector<ItemElementType>& leftVector = dataset.getDataVector(leftId); 
                    const vector<ItemElementType>& rightVector = dataset.getDataVector(rightId); 
                    float dist = distor(leftVector, rightVector);

                    pair<int, float> msgToLeft(rightId, dist);
                    result_channel.push(msgToLeft, leftId);

                    pair<int, float> msgToRight(leftId, dist);
                    result_channel.push(msgToRight, rightId);
                }
            }
        });

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finished local join in " << timer.getDelta() << " seconds" << std::endl;
    }
    // update adj_list by result_channel
    husky::list_execute(
        adj_list,
        {&result_channel},
        {},
        [&result_channel](AdjObject& adj){
        const vector<pair<int, float>>& msgs = result_channel.get(adj);
        if (msgs.size() > 0)
            adj.mergeFoundKNN(msgs);
    });

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finished merge new candidates in " << timer.getDelta() << " seconds" << std::endl;
    }

    AdjObject::updateRKNN(adj_list);
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finished updateRKNN in " << timer.getDelta() << " seconds" << std::endl;
    }
}

thread_local husky::PushChannel<int, AdjObject>* AdjObject::_Adj2AdjPushIntCH = NULL;

}
}
