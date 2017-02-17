#pragma once
#include <string>
#include <set>
#include <bitset>

#include "base/log.hpp"
#include "io/hdfs_manager.hpp"

#include "DenseVector.hpp"
#include "LSHFactory.hpp"
#include "LSHItem.hpp"
// template<typename ItemIdType, typename ItemElementType>
// class PLSHItem : public LSHItem<ItemIdType, ItemElementType> {
// public:
//     virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory) {
//
//        // std::set<ItemIdType> evaluated;
//         std::bitset<1000> evaluated;
//         for(auto& query : Husky::get_messages<
//                 DenseVector<ItemIdType, ItemElementType> >(*this) ) {
//             //
//             // if(evaluated.find( query.getItemId() ) != evaluated.end() ) continue;
//             // evaluated.insert(query.getItemId());
//             
//             if(evaluated[ query.getItemId() ] == 1) continue;
//             evaluated.set( query.getItemId(), 1) ;
//
//             float distance = factory.calDistance(query.getItemVector(), this->getItemVector());
//
//             // if(distance <= 0.9) {
//             //     std::string result;
//             //     result += std::to_string( query.getItemId() ) + " ";
//             //     result += std::to_string( this->getItemId() ) + " " + std::to_string(distance) + "\n";
//             //     this->write_to_hdfs( result , LSHContext::getOutputPath() );
//             // }
//         }
//     }
// };
//
// for broadcast
// template<typename ItemIdType, typename ItemElementType>
template<typename ItemIdType = int, typename ItemElementType = std::pair<int, float> >
class PLSHItem : public LSHItem<ItemIdType, ItemElementType, ItemIdType> {
	public:

		virtual void answer(LSHFactory<ItemIdType, ItemElementType>& factory) {

			std::set<ItemIdType> evaluated;

			// // std::bitset<1000> evaluated;
			// // for(auto& queryId : Husky::get_messages< ItemIdType >(*this) ) {
			// for(auto& queryId : getQueryMsg() ) {
			// 	//
			// 	if(evaluated.find( queryId ) != evaluated.end() ) continue;
			// 	evaluated.insert(queryId );
            //
			// 	// if(evaluated[queryId] == 1) continue;
			// 	// evaluated.set(queryId, 1) ;
            //
			// 	//broadcast will halt the string key
			// 	auto& queryVector = this->template get_response<
			// 		std::vector<ItemElementType> >(queryId);
			// 	float distance = factory.calDistance(queryVector, this->getItemVector());
            //
			// 	if(distance <= 0.9) {
			// 		std::string result;
			// 		result += std::to_string( queryId ) + " ";
			// 		result += std::to_string( this->getItemId() ) + " " + std::to_string(distance) + "\n";
			// 		Husky::HDFS::Write( Husky::Context::get_params("hdfs_namenode"), Husky::Context::get_params("hdfs_namenode_port"), result, LSHContext::getOutputPath(), get_worker().id );
			// 	}
			// }

		}
};
