#pragma once
#include <vector>
#include <utility>
#include <random>
#include <unordered_set>
#include "dataobject.h"
#include "knnagg.h"
using std::vector;
using std::pair;

namespace husky {
namespace losha {

class AdjObject{
public:
    using KeyT = int;
    KeyT _vid;
    vector<pair<int, float>> _trueKNN;
    vector<pair<int, float>> _foundKNN;

    vector<int> _rKNN;
    int _label;

    explicit AdjObject(const KeyT& id) : _vid(id), _label(id) {};
    const KeyT id() const {return _vid;};


    void setTrueKNN(const vector<pair<int, float>>& trueKNN) {
        _trueKNN = trueKNN;
    }

    static void initFromLSHBOX(
        const string& lshboxPath, 
        husky::ObjList<AdjObject>& adj_list,
        int maxItemId) {
        loadSampleGroundtruth(lshboxPath, adj_list);
        husky::list_execute(
            adj_list,
            [&maxItemId](AdjObject& adj) {
                adj.randomInitFoundKNN(maxItemId);
            });
        updateRKNN(adj_list);
    }

    static void updateRKNN(
        husky::ObjList<AdjObject>& adj_list);

    static void clustering(
        husky::ObjList<AdjObject>& adj_list, 
        int numHops);
private:
    template<typename ChannelType>
    void propagateLabel(ChannelType& ch) {
        for (const auto& p : _foundKNN) {
            if (p.first > _label) 
                ch.push(_label, p.first);
        }
        for (auto rNB : _rKNN) {
            if (rNB > _label) {
                ch.push(_label, rNB);
            }
        }
    }
    static void loadSampleGroundtruth(
        const string& inputPath, 
        husky::ObjList<AdjObject>& obj_list);

    void randomInitFoundKNN(int maxItemId) {
        std::default_random_engine generator(_vid);
        std::uniform_int_distribution<unsigned> distribution(0, maxItemId - 1);

        std::unordered_set<unsigned> inserted;
        float maxDist = std::numeric_limits<float>::max();

        while(_foundKNN.size() < _trueKNN.size()) {
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
     
    auto& channel = 
        husky::ChannelStore::create_push_channel<int>(adj_list, adj_list);

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

void AdjObject::clustering(
    husky::ObjList<AdjObject>& adj_list, 
    int numHops) {

    // cal cc until there are approximate cc blocks 
    auto& msgCh =
            husky::ChannelStore::create_push_combined_channel<int, husky::MinCombiner<int>>(adj_list, adj_list);
    husky::list_execute(
        adj_list,
        {},
        {&msgCh},
        [&msgCh](AdjObject& adj){
        adj.propagateLabel(msgCh);
    });

    for (int i = 0; i < numHops; ++i) {
        husky::list_execute(
            adj_list,
            {&msgCh},
            {&msgCh},
            [&msgCh](AdjObject& adj){
            if (msgCh.has_msgs(adj) 
                && msgCh.get(adj) < adj._label) {
                adj._label = msgCh.get(adj);
                adj.propagateLabel(msgCh);
            }
        });
    }

    // report number of clusters 

    vector<pair<int, int>> agg =
        keyValueAgg<AdjObject, int, int, husky::SumCombiner<int>>(
            adj_list,
            [](const AdjObject& v){ return v._label;},
            [](const AdjObject& v){ return 1;});
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "number of clusters is " << agg.size() << std::endl;
        husky::LOG_I << "present cluster_id and size in (cluster_id, size)" << std::endl;
        // for (int i = 0; i < agg.size(); ++i) {
        //     husky::LOG_I << "(" << agg[i].first << ", " << agg[i].second << ")" << std::endl;
        // }
    }
    // calNumBlocks and size
    // husky::list_execute(adj_list, [](AdjObject& v) {
    //     husky::LOG_I << "adj: " << v.id() << " component id: " << v._label << std::endl;
    // });
}
}
}
