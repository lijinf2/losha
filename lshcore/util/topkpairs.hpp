/*
 * Copyright 2016 Husky Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implement a heap with size k to maintain top-k items */
#pragma once
#include <utility>
#include <queue>

namespace husky {
namespace losha {
/*
 * Insert <id, distance> pairs, order pairs by distance and output k pairs with minimum distance*/
template<typename IdType, typename DistType>
class TopkPairs{
public:
    TopkPairs(int maxSize) {
        _maxSize = maxSize;
    };
    void push(const std::pair<IdType, DistType>& p);
    // return all pairs ordered by distance, and remove them from heap 
    std::vector<std::pair<IdType, DistType>> popAll();

private:
    class Comparator{
    public:
        bool operator()(
            const std::pair<IdType, DistType>& a,
            const std::pair<IdType, DistType>& b) {
            return a.second < b.second;
        }
    };

    std::priority_queue<
        std::pair<IdType, DistType>,
        std::vector<std::pair<IdType, DistType>>,
        Comparator> _maxHeap;
    int _maxSize;
};

template<typename IdType, typename DistType>
void TopkPairs<IdType, DistType>::push( const std::pair<IdType, DistType>& p) {
    _maxHeap.push(p);
    if (_maxHeap.size() > _maxSize) {
        _maxHeap.pop();
    }
}

template<typename IdType, typename DistType>
std::vector<std::pair<IdType, DistType>> TopkPairs<IdType, DistType>::popAll() {
    std::vector<std::pair<IdType, DistType>> vec;
    vec.resize(_maxHeap.size());
    for(int i = vec.size() - 1; i >=0; --i){
        vec[i] = _maxHeap.top();
        _maxHeap.pop();
    }
    return vec;
}

} // namespace losha
} // namespace husky
