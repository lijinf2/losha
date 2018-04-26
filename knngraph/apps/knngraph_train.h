#pragma once
#include "core/engine.hpp"
class CCVertex {
public:
    using KeyT = int;
    KeyT _vid;
    explicit CCVertex(const KeyT& id) : _vid(id), _label(id) {};
    const KeyT& id() const {return _vid}

    void setNeighbors(const vector<int>& nbs) {
        _nbs = nbs;
    };
private:
    int _label;
    vector<int> _nbs;
};
template<typename FeatureType>
void clustering(
    husky::ObjList<KNNVertex<FeatureType>>& item_list, 
    husky::ObjList<Block<FeatureType>>& block_list) {

    auto & ccvertex_list =
        husky::ObjListStore::create_objlist<CCVertex>();

    auto& channel = 
        husky::ChannelStore::create_push_channel<
           vector<int>>(item_list, ccvertex_list);

    husky::list_execute(item_list,
        [&channel](KNNVertex<FeatureType>& item){
        channel.push(item.getNeighbors(), item.getItemId());
    });

    husky::list_execute(ccvertex_list,
        [&channel](CCVertex<FeatureType>& vertex){
        auto msg = channel.get(vertex);
        assert(msg.size() == 1)
        vertex.setNeighbors(msg[0]);
    });

    // cal cc until there are approximate cc blocks 
}
