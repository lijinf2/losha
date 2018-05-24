// template cache, calculate connected component of adj_list
void AdjObject::clustering(
    husky::ObjList<AdjObject>& adj_list, 
    int numHops) {

    // cal cc until there are approximate cc blocks 
    
    auto& msgCh =
            husky::ChannelStore::create_push_combined_channel<int, husky::MinCombiner<int>>(adj_list, adj_list);
    husky::list_execute(
        adj_list,
        {},
        {&msgCh},
        [&msgCh](AdjObject& adj){
        adj.propagateLabel(msgCh);
    });

    for (int i = 0; i < numHops; ++i) {
        husky::list_execute(
            adj_list,
            {&msgCh},
            {&msgCh},
            [&msgCh](AdjObject& adj){
            if (msgCh.has_msgs(adj) 
                && msgCh.get(adj) < adj._label) {
                adj._label = msgCh.get(adj);
                adj.propagateLabel(msgCh);
            }
        });
    }

    // report number of clusters 

    vector<pair<int, int>> agg =
        keyValueAgg<AdjObject, int, int, husky::SumCombiner<int>>(
            adj_list,
            [](const AdjObject& v){ return v._label;},
            [](const AdjObject& v){ return 1;});
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "number of clusters is " << agg.size() << std::endl;
        husky::LOG_I << "present cluster_id and size in (cluster_id, size)" << std::endl;
        std::sort(
            agg.begin(),
            agg.end(),
            [](const pair<int, int>& a, const pair<int, int>&b){
                return a.second > b.second;
            });
        for (int i = 0; i < agg.size(); ++i) {
            husky::LOG_I << "(" << agg[i].first << ", " << agg[i].second << ")" << std::endl;
        }
    }
}
