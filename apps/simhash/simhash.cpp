#include <boost/tokenizer.hpp>
#include <cmath>
#include <vector>
#include <limits>
#include <chrono>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshutils.hpp"
#include "lshcore/lshengine.hpp"
#include "lshcore/SimHashFactory.hpp"
#include "simhash.hpp"

/**************************
// This part can handle input from:
// 1. ./small_dataset/
// Input is in .txt format
****************************/
//#include "basic.hpp"

/**************************
// This part can handle glove2.2M input, downloaded from: http://nlp.stanford.edu/projects/glove/
// (Common Crawl: 840B tokens, 2.2M vocab, cased, 300d vectors)
****************************/
#include "glove2m.hpp"

using namespace husky::losha;
SimHashFactory<int, float> factory;
std::once_flag factory_flag;

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

    // INPUT: .txt format
    //auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();
    //loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, lineInputFormat);
    
    // INPUT: .bin format
    auto& binaryInputFormat = 
        husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector);
    loshaengine<FinalSimhashQuery, FinalSimhashBucket, FinalSimhashItem, QueryMsg, AnswerMsg>(factory, setItem, binaryInputFormat);

    auto query_f = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_query = query_f - init_f;
    if(husky::Context::get_global_tid() == 0)
        husky::LOG_I << "Job query finishes in "  
                     <<  std::to_string( d_query.count() / 1000.0)
                     << " seconds" << std::endl;
    if(husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "Finish SimHash" << std::endl;
}

int main(int argc, char ** argv) {
    husky::LOG_I << "SimHash program starts" << std::endl;
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("band");
    args.push_back("row");
    args.push_back("dimension");
    args.push_back("iters");
    args.push_back("queryPath"); 
    args.push_back("itemPath"); 
    args.push_back("outputPath"); 
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}

