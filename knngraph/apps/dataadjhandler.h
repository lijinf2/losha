#include <string>
#include "core/engine.hpp"
#include "dataobject.h"
#include "adjobject.h"
using std::string;

namespace husky {
namespace losha {
class DataAdjHandler {
public:
    template<typename ItemElementType>
    static void buildAdjFromData(
        husky::ObjList<DataObject<ItemElementType>>& data_list,
        husky::ObjList<AdjObject>& adj_list,
        const string& lshboxPath,
        int maxItemId) {

        // create adj_list
        
        auto& channel = 
            husky::ChannelStore::create_push_channel<
               int>(data_list, adj_list);
        husky::list_execute(
            data_list,
            {},
            {&channel},
            [&channel](DataObject<ItemElementType>& data) {
                channel.push(data.id(), data.id());
            });

        husky::list_execute(
            adj_list,
            {&channel},
            {},
            [&channel](AdjObject& adj) {
                assert(channel.get(adj).size() == 1);
            });

        // initilize adj_list, load groundtruth and random assign nbs
        AdjObject::initFromLSHBOX(lshboxPath, adj_list, maxItemId);

    }

};
}
}
