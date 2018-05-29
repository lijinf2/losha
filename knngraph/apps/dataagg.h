#pragma once
#include <vector>
#include "core/engine.hpp"
#include "lib/aggregator_factory.hpp"

#include "losha/common/aggre.hpp"
#include "dataobject.h"

using std::vector;
namespace husky {
namespace losha {

// assume data id ranges from 0 to n - 1
template<typename FeatureType>
class DataAgg {
public:
    typedef vector<FeatureType> RecordT;
    DataAgg(
        husky::ObjList<DataObject<FeatureType>>& data_list,
        int numItems) {

        int numAggs = husky::Context::get_num_processes(); 
        int numExpectedItems = numItems / numAggs + 1; 
        _expectedItemsPerAgg = numExpectedItems;

        auto id = husky::Context::get_global_tid();
        int acc = 0;
        while(acc < numItems) {
            int actualItems = numExpectedItems;
            if (acc + actualItems > numItems) {
                actualItems = numItems - acc;
            } 
            auto agg = this->genVecAgg(actualItems);
            _aggs.push_back(agg);
            _numVectorEachAgg.push_back(actualItems);
            acc += actualItems;
        }

        husky::list_execute(
            data_list, 
            {},
            {},
            [this, numExpectedItems](DataObject<FeatureType>& data){

                auto& collector = this->_aggs[data.id() / numExpectedItems];

                collector.update(
                    [&data, &numExpectedItems](vector<RecordT>& collection, const RecordT& record){
                        assert(data.id() % numExpectedItems < collection.size());
                        collection[data.id() % numExpectedItems] = record;
                    },
                    data.getItemVector());
        });
        husky::lib::AggregatorFactory::sync();  
    }

    RecordT& getDataVector(int id) {
        auto& collection = _aggs[id / _expectedItemsPerAgg].get_value();
        return collection[id % _expectedItemsPerAgg];
    }

protected:
    vector<husky::lib::Aggregator<vector<RecordT>>> _aggs;
    vector<int> _numVectorEachAgg;
    int _expectedItemsPerAgg = -1;

    husky::lib::Aggregator<vector<RecordT>> genVecAgg(int size) {

        husky::lib::Aggregator<vector<RecordT>> agg(
            vector<RecordT> (size),
            [](vector<RecordT>& a, const vector<RecordT>& b){
                for (int i = 0; i < b.size(); ++i) {
                    if (b[i].size() != 0) {
                        assert(a[i].size() == 0);
                        a[i] = b[i];
                    }
                }
            },
            [size](vector<RecordT>& col){
                col = vector<RecordT> (size);
            });

        return agg; 
    }
};

}
}
