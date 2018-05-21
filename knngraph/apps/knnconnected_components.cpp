#include <vector>
#include <string>
#include <iostream>
#include "boost/tokenizer.hpp"

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "losha/common/writer.hpp"
#include "knnconnected_components.h"
using std::vector;
using std::string;
using namespace husky::losha;

class CCVertex {
public:
    using KeyT = int;
    KeyT _vid;
    vector<int> _nbs;
    int _label; 
    explicit CCVertex(int id) : _vid(id), _label(id) {
        _vid = id;
    }
    const KeyT id() const {return _vid;};

};

void knnconnected_components() {
    string graphPath = husky::Context::get_param("knngraph_path");
    auto& lineInputformat =
        husky::io::InputFormatStore::create_line_inputformat();
    lineInputformat.set_input(graphPath);

    auto & vertex_list =
        husky::ObjListStore::create_objlist<CCVertex>();

    auto& channel = 
        husky::ChannelStore::create_push_channel<int>(lineInputformat, vertex_list);

    auto parser = [&channel](boost::string_ref& line) {
        boost::char_separator<char> sep(" \t");
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);

        int queryId;
        int itemId;
        float distance;

        auto it = tok.begin();
        queryId = std::stoi(*it++);

        while(it != tok.end()) {
            itemId = std::stoi(*it++);
            distance = std::stof(*it++);
            channel.push(itemId, queryId);
            channel.push(queryId, itemId);
        }
    };
    husky::load(lineInputformat, parser);

    husky::list_execute(
        vertex_list,
        {&channel},
        {},
        [&channel](CCVertex& vertex){
            vertex._nbs = channel.get(vertex);
            // auto msgs = channel.get(vertex);
            // assert(msgs.size() == 1);
            // vertex._nbs = msgs[0];

            string line = std::to_string(vertex.id());
            line += " ";
            line += std::to_string(vertex._nbs.size());
            line += "\n";
            writeHDFS(line, "hdfs_namenode", "hdfs_namenode_port", "output_path");
        });

    int numNodes = count(vertex_list);
    auto getLabelRef = 
        [](CCVertex& vertex) -> int&{
            return vertex._label;
        };

    auto getNeighborsRef =  
        [](CCVertex& vertex) -> const vector<int>& {
            return vertex._nbs;
        };

    KNNConnectedComponents::compute<CCVertex>(
        vertex_list,
        getLabelRef,
        getNeighborsRef);

    auto blockToSize = 
        KNNConnectedComponents::getClusteringDesc<CCVertex>(
           vertex_list, 
           [](const CCVertex& vertex){ return vertex._label;});

    KNNConnectedComponents::reportClustering(blockToSize);
}

int main(int argc, char ** argv) {
    std::cout << "start to run program knnconnected_components" << std::endl;
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("knngraph_path"); 
    args.push_back("output_path"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(knnconnected_components);
        return 0;
    }
    return 1;
}
