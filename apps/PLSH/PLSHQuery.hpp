#pragma once
#include "LSHQuery.hpp"
// #include "LSHUtils.hpp"
// #include <algorithm>
// #include <base/log.hpp>

using namespace husky::losha;
// template<typename ItemIdType, typename ItemElementType>
// class PLSHQuery : public LSHQuery<ItemIdType, ItemElementType> {
// public:
//
//     std::vector< std::vector<int> > buckets;
//     virtual void setup(LSHFactory<ItemIdType, ItemElementType> &factory) override {
//         this->buckets =
//             factory.calItemBuckets(this->getQuery());
//         this->queryMsg = this->getQuery();
//         // Husky::log_msg("query id: " + this->getItemId() + " and vectors size: " 
//         //         +std::to_string(this->getItemVector().size() ));
//         // this->queryMsg = this->getItemId();
//         // broadcast(*this, this->getItemVector() );
//     }
//
//     virtual void query( LSHFactory<ItemIdType, ItemElementType>& factory) override {
//
//         std::vector< std::vector<int> >::iterator it;
//         for( it = this->buckets.begin() ; it != this->buckets.end(); ++it) {
//             // get_worker().send_message( queryPoint, *it, bucket_list);
//             this->sendToBucket(*it);
//         }
//     }
// };

// for broadcasting
template<typename ItemIdType, typename ItemElementType>
class PLSHQuery : public LSHQuery<ItemIdType, ItemElementType, ItemIdType> {
public:

    virtual void query( LSHFactory<ItemIdType, ItemElementType>& factory) override {
        std::vector< std::vector<int> > buckets = 
            factory.calItemBuckets(this->getQuery());

        this->queryMsg = this->getItemId();
        
        // broadcast content
        this->broadcast();

        std::vector< std::vector<int> >::iterator it;
        for( buckets.begin() ; it != buckets.end(); ++it) {
            // get_worker().send_message( queryPoint, *it, bucket_list);
            this->sendToBucket(*it);
        }

    }
};
