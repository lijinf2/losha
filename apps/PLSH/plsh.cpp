#include <boost/tokenizer.hpp>
#include <cmath>
#include <vector>
#include <limits>
#include <chrono>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshutils.hpp"
#include "lshcore/lshengine.hpp"
#include "lshcore/sparse/apsparsecoslshfactory.hpp"
#include "plsh.hpp"

using namespace husky::losha;
APSparseCosLSHFactory<int, float> factory;
std::once_flag factory_flag;

// input format : id f1:v1 f2:v2 f3:v3 ...
void setItem(
    boost::string_ref& line,
    int& itemId,
    std::vector<std::pair<int, float> >& itemVector) {

    assert(line.size() != 0);

    boost::char_separator<char> sep(" \t");
    boost::tokenizer<boost::char_separator<char>> tok(line, sep);

    //optimize to use dimension to avoid push back set lshStof
    bool setId = false;
    for(auto &w : tok) {
        if(!setId) {
            itemId = std::stoi(w);
            setId = true;
        } else itemVector.push_back( lshStofPair(w) );
        // else itemVector.push_back( std::stof(w) );
    }
    assert (itemVector.size() != 0);
}

void lsh() {

    auto start_s = std::chrono::steady_clock::now();

    // initialization
    int band = std::stoi(husky::Context::get_param("band"));
    int row = std::stoi(husky::Context::get_param("row"));
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    std::call_once(factory_flag, [&]() {
        factory.initialize(band, row, dimension);
    });

    auto init_f = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_init = init_f - start_s;
    if(husky::Context::get_global_tid == 0)
        husky::LOG_I << "Job init finishes in " 
                     << std::to_string( d_init.count() / 1000.0 ) 
                     << " seconds" << std::endl;

    auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();


    loshaengine<PLSHQuery, PLSHBucket, PLSHItem, QueryMsg, AnswerMsg>(
        factory, setItem, lineInputFormat);

    auto query_f = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_query = query_f - init_f;
    //free worker you can not do anything after free worker
    if(husky::Context::get_global_tid() == 0)
        husky::LOG_I << "Job query finishes in "  
                     <<  std::to_string( d_query.count() / 1000.0)
                     << " seconds" << std::endl;
    if(husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "finish plsh" << std::endl;
}

int main(int argc, char ** argv) {
    husky::LOG_I << "program starts" << std::endl;
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("band");
    args.push_back("row");
    args.push_back("dimension");
    args.push_back("iters");
    args.push_back("queryPath"); // the inputPath
    args.push_back("itemPath"); // the outputPath
    args.push_back("outputPath"); // the inputPath
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}

