#include <vector>
#include <string>
#include "core/engine.hpp"
#include "knngraph_train.h"
#include "losha/common/distor.hpp"

using std::vector;
using std::string;
using namespace husky::losha;

void knngraph_train_runner() {
    // initialization
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::string itemPath = husky::Context::get_param("item_path");
    std::string sampleGroundtruthPath = husky::Context::get_param("sample_groundtruth_path");
    int maxIteration = std::stoi(husky::Context::get_param("max_iteration")); 
    int numBlocks = std::stoi(husky::Context::get_param("num_block"));
    // int numHops = std::stoi(husky::Context::get_param("num_hop"));
    int numNBPerNode = std::stoi(husky::Context::get_param("K"));

    knngraph_train<float>(
        dimension,
        itemPath, 
        sampleGroundtruthPath,
        maxIteration,
        numBlocks,
        numNBPerNode,
        calE2Dist);
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("dimension");
    args.push_back("item_path"); 
    args.push_back("sample_groundtruth_path"); 
    args.push_back("max_iteration"); 
    args.push_back("tmp_output_path"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(knngraph_train_runner);
        return 0;
    }
    return 1;
}
