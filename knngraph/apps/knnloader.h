#pragma once
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include "core/engine.hpp"
#include "boost/tokenizer.hpp"
#include "dataobject.h"
#include "block.h"
using std::string;
using std::vector;
using std::pair;
using std::istringstream;

namespace husky {
namespace losha {

template<typename FeatureType>
void initBlocks(
    int blockPerThread, 
    husky::ObjList<Block<FeatureType>>& block_list) {
    
    auto numWorkers = husky::Context::get_num_workers();
    int blockId = husky::Context::get_global_tid();
    for (int i = 0; i < blockPerThread; ++i) {
        block_list.add_object(Block<FeatureType>(blockId));
        blockId += numWorkers;
    }
    husky::globalize(block_list);
}
}
}
