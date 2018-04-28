#include <string>

#include "base/serialization.hpp"
#include "base/log.hpp"
#include "base/thread_support.hpp"
#include "boost/functional/hash.hpp"
#include "core/combiner.hpp"
#include "core/engine.hpp"
#include "io/input/line_inputformat.hpp"
#include "lib/aggregator_factory.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/loader/loader.h"
#include "lshcore/lshutils.hpp"
#include "lshcore/lshengine.hpp"
#include "losha/common/distor.hpp"


namespace husky {
    namespace losha {
        
        template<typename FirstType, typename SecondType>
        class PairComparison {
        public: 
            typedef std::pair<FirstType, SecondType> pii;
            bool operator () (const pii &n1, const pii &n2) const {
                if (n1.second == n2.second) {
                    return n1.first > n2.first;

                }
                return n1.second < n2.second;
            }

        };

        template<typename QueryType, typename ItemType, typename ItemIdType, typename ItemElementType>
        class LinearScanner {
            typedef std::pair<ItemIdType, ItemElementType> AnswerMsg;
            typedef std::priority_queue<AnswerMsg, vector<AnswerMsg>, PairComparison<ItemIdType, ItemElementType > > TopKQueueType;
        public:
            void linearScan() { 
                int dimension = std::stoi(husky::Context::get_param("dimension"));
                int topK = std::stoi(husky::Context::get_param("topK"));
            
                std::string itemPath = husky::Context::get_param("itemPath");
                std::string queryPath = husky::Context::get_param("queryPath");

                int sizeOfVector = dimension * 4 + 8;
            
                void (*setItem)(boost::string_ref&, ItemIdType&, vector<ItemElementType>&) = parseIdFvecs;

                if (husky::Context::get_global_tid() == 0) {
                    husky::LOG_I << "dimension   : " << dimension << std::endl;
                    husky::LOG_I << "topK        : " << topK << std::endl;
                    husky::LOG_I << "item        : " << itemPath << std::endl;
                    husky::LOG_I << "quey        : " << queryPath << std::endl;
                    husky::LOG_I << "vector size : " << sizeOfVector << std::endl;
                }

                auto& infmt = husky::io::InputFormatStore::create_chunk_inputformat(sizeOfVector); 

                infmt.set_input(itemPath);
                auto & item_list = husky::ObjListStore::create_objlist<ItemType>();
                loadQueries(item_list, setItem, infmt);

                husky::list_execute(item_list, [](ItemType& item){
                        });
                std::cout << "item size  : " << item_list.get_size() << std::endl;

                infmt.set_input(queryPath); 
                auto & query_list = husky::ObjListStore::create_objlist<QueryType>();
                loadQueries(query_list, setItem, infmt);
                
                husky::list_execute(query_list, [](QueryType& query) {
                        });
                std::cout << "query size : " << query_list.get_size() <<std::endl;

                if (husky::Context::get_global_tid() == 0)
                    husky::LOG_I << "[broadcast] start" << std::endl;
                
                std::function<void(ItemIdType&, std::vector<ItemElementType>& )> query_handler =
                    [](ItemIdType& first, std::vector<ItemElementType>& second){
                        insertQueryVector(first, second);
                    };
                
                broadcastQueries(query_handler, query_list);

                std::cout << "query map size : " << getIdToQueryMap().size() <<std::endl;

                if (husky::Context::get_global_tid() == 0)
                    husky::LOG_I << "[calculate] start calculate distance and send distance to queries" << std::endl;

                auto& item2queryChannel = husky::ChannelStore::create_push_channel<
                        AnswerMsg>(item_list, query_list);


                auto& _idToQueryVector = getIdToQueryMap();

                husky::list_execute(
                        item_list,
                        [&](ItemType& item) {
                            for (auto iter = _idToQueryVector.begin(); iter!=_idToQueryVector.end(); iter++) {
                                ItemElementType item_query_distance = calE2Dist(item.getItemVector(), iter->second);
                                item2queryChannel.push(std::make_pair(item.id(), item_query_distance), iter->first);
                            }
                        }
                );

                if (husky::Context::get_global_tid() == 0)
                    husky::LOG_I << "[receive] receive distance and choose topK" << std::endl;
                
                husky::list_execute(
                        query_list,
                        [&](QueryType& query) {
                           
                            TopKQueueType& topKQueue = topKQueues[query.id()];
                            
                            for (auto& msg : item2queryChannel.get(query)) {
                                ItemIdType itemId = msg.first;
                                ItemElementType item_query_distance = msg.second;

                                if (topKQueue.size() < topK ) {
                                
                                    topKQueue.push(std::make_pair(itemId, item_query_distance));
                                } else if (topKQueue.top().second > item_query_distance) {
                                    topKQueue.pop();
                                    topKQueue.push(std::make_pair(itemId, item_query_distance));
                                }
                            }
                        }
                );
                for (auto iter = topKQueues.begin(); iter != topKQueues.end(); ++iter) {
                    ItemIdType queryId = iter->first;
                    TopKQueueType& topKRecords = iter->second;
                    while(topKRecords.size()) {
                        const std::pair<ItemIdType, ItemElementType>& topKRecord = topKRecords.top();
                        writeHDFSTriplet(queryId, topKRecord.first, topKRecord.second, "hdfs_namenode", "hdfs_namenode_port", "outputPath");
                        topKRecords.pop();
                    }
                }
                //end here
            }

        private:

            std::unordered_map<ItemIdType, TopKQueueType > topKQueues;

            static std::unordered_map<ItemIdType, std::vector<ItemElementType> >& getIdToQueryMap() {
                static std::unordered_map<ItemIdType, std::vector<ItemElementType> > _idToQueryVector;
                return _idToQueryVector;
            }

            // Fetch broadcasted queries into _idToQueryVector
            static inline void insertQueryVector(int qid, const std::vector<ItemElementType>& qvec) {
                auto& _idToQueryVector = getIdToQueryMap();
                if (_idToQueryVector.find(qid) != _idToQueryVector.end()) {
                    //ASSERT_MSG(0, "query already exists");
                }
                _idToQueryVector[qid] = qvec;
            }


     };

    };
};
