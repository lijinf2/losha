/*
 * Copyright 2016 Husky Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cmath>
#include <limits>
#include <vector>

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include "lshcore/lshengine.hpp"
#include "lshcore/e2lshfactory.hpp"

#include "sift_ir.hpp"

using namespace husky::losha;
E2LSHFactory<ItemIdType, ItemElementType> factory;
std::once_flag factory_flag;

void lsh() {
    auto start_s = std::chrono::steady_clock::now();

    // initialization
    int band = std::stoi(husky::Context::get_param("band"));
    int row = std::stoi(husky::Context::get_param("row"));
    int dimension = std::stoi(husky::Context::get_param("dimension"));
    int W = std::stoi(husky::Context::get_param("W"));
    std::call_once(factory_flag, [&]() {
        factory.initialize(band, row, dimension, W);
    });

    auto init_f = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_init = init_f - start_s;
    if (husky::Context::get_global_tid() == 0)
        husky::LOG_I << "Job init finishes in "
            << std::to_string(d_init.count() / 1000.0) 
            << " seconds" << std::endl;

    // Small Dataset (txt file)
    // auto& lineInputFormat = husky::io::InputFormatStore::create_line_inputformat();        
    // loshaengine<Query1B, Bucket1B, Item1B, QueryMsg, AnswerMsg>(
    //    factory, setItemSMALL, lineInputFormat);

    // Small Dataset (binary file)
    //auto& binaryInputFormat = 
    //    husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    //loshaengine<Query1B, Bucket1B, Item1B, QueryMsg, AnswerMsg>(
    //    factory, setItemSmallBinary, binaryInputFormat);

    //IR
    auto& binaryInputFormat = 
       husky::io::InputFormatStore::create_chunk_inputformat(BytesPerVector); 
    loshaengine<Query1B, Bucket1B, Item1B, QueryMsg, AnswerMsg>(
       factory, setItemSIFT1B, binaryInputFormat);

    auto query_f = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> d_query = query_f - init_f;
    if(husky::Context::get_global_tid() == 0)
        husky::LOG_I << "Job query finishes in "  
                     <<  std::to_string( d_query.count() / 1000.0)
                     << " seconds" << std::endl;
    if(husky::Context::get_global_tid() == 0) 
        husky::LOG_I << "E2LSH finish" << std::endl;
}

int main(int argc, char ** argv) {
    husky::LOG_I << "E2LSH program starts" << std::endl;
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("band");
    args.push_back("row");
    args.push_back("dimension");
    args.push_back("queryPath"); // the inputQueryPath
    args.push_back("itemPath"); // the inputItemPath
    args.push_back("outputPath"); // the outputPath
    args.push_back("W");
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(lsh);
        return 0;
    }
    return 1;
}

