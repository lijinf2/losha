#pragma once
#include "lshcore/densevector.hpp"
#include "core/engine.hpp"
#include <utility>
#include <vector>
#include <limits>
using std::pair;
using std::vector;
namespace husky {
namespace losha {

// Optimization: separate data and knn for better cache locality
// data vertex
template<typename ItemElementType>   
class DataObject : public DenseVector<int, ItemElementType>{
public:
    explicit DataObject(
        const typename DataObject::KeyT& id) : DenseVector<int, ItemElementType>(id) {};
    DataObject() : DenseVector<int, ItemElementType>(){}

    static void loadIdFvecs(
        husky::ObjList<DataObject<ItemElementType>>& obj_list,
        const string& itemPath, 
        int dimension);
};

template<typename ItemElementType>
void DataObject<ItemElementType>::loadIdFvecs(
    husky::ObjList<DataObject<ItemElementType>>& obj_list,
    const string& itemPath, 
    int dimension) {

    int BytesPerVector = dimension * sizeof(ItemElementType) + 8;
    auto& binaryInputFormat = husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    binaryInputFormat.set_input(itemPath);

    auto& loadChannel = 
        husky::ChannelStore::create_push_channel<
           vector<ItemElementType>>(binaryInputFormat, obj_list);
    husky::load(binaryInputFormat, item_loader(loadChannel, parseIdFvecs<ItemElementType>));

    husky::list_execute(obj_list,
        [&loadChannel, &dimension](DataObject<ItemElementType>& obj){
        auto msgs = loadChannel.get(obj);
        assert(msgs.size() == 1);
        obj.setItemVector(msgs[0]);
        assert(obj.getItemVector().size() == dimension);
    });

    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "finish loading from itemPath: " << itemPath << std::endl;
    }
}
}
}
