#pragma once
#include "core/engine.hpp"
#include "lib/aggregator_factory.hpp"

namespace husky {
namespace losha {
template<typename ObjT>
unsigned count(
    husky::ObjList<ObjT>& obj_list) {

    husky::lib::Aggregator<unsigned> countAgg(0,
            [](unsigned& a, const unsigned& b){  a += b; });

    auto& ac = husky::lib::AggregatorFactory::get_channel();
    husky::list_execute(obj_list, {}, {&ac}, 
        [&countAgg](ObjT& obj) {
        countAgg.update(1);
     });
    husky::lib::AggregatorFactory::sync();
    unsigned size = countAgg.get_value();
    return size;
}

}
}
