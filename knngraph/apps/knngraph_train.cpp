#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <utility>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/densevector.hpp"
#include "lshcore/loader/loader.h"

using namespace husky::losha;
using std::vector;
using std::pair;
using std::string;

void knngraph_train() {

    // initialization
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::string itemPath = husky::Context::get_param("item_path");
    std::string sampleGroundtruthPath = husky::Context::get_param("sample_groundtruth_path");

    typedef float FeatureType;
    // load items
    auto & item_list =
        husky::ObjListStore::create_objlist<DenseVector<int, FeatureType>>();

    loadIdFvecs(item_list, itemPath, dimension);

}

int main(int argc, char ** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("dimension");
    args.push_back("item_path"); 
    args.push_back("sample_groundtruth_path"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(knngraph_train);
        return 0;
    }
    return 1;
}
