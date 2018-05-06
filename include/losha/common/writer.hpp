#pragma once
#include<string>
#include <vector>
#include<utility>
#include "core/engine.hpp"
#include "io/hdfs_manager.hpp"

using std::string;
using std::pair;
using std::vector;

namespace husky {
namespace losha {

void writeHDFS(const string& text, const string& namenodeKey, const string& portKey, const string& outputPathKey) {
    husky::io::HDFS::Write(
        husky::Context::get_param(namenodeKey),
        husky::Context::get_param(portKey),
        text,
        husky::Context::get_param(outputPathKey),
        husky::Context::get_global_tid());
}

template<typename ItemIdType>
void writeHDFSTriplet(
    const ItemIdType& queryId, const ItemIdType& itemId, float distance,
    const string& namenodeKey, const string& portKey, const string& outputPathKey) {
    string text = std::to_string(queryId) + " ";
    text += std::to_string(itemId) + " " + std::to_string(distance) + "\n";
    writeHDFS(text, namenodeKey, portKey, outputPathKey);
}

template<typename ElementType>
void writeHDFSVector(
    const vector<ElementType>& vec,
    const string& namenodeKey, const string& portKey, const string& outputPathKey) {
    string text = "";
    for (int i = 0; i < vec.size(); ++i) {
        text += " ";
        text += std::to_string(vec[i]);
    }
    writeHDFS(text, namenodeKey, portKey, outputPathKey);
}

template<typename ItemIdType, typename DistType>
void writeHDFSTriplet(
    const ItemIdType& queryId, const std::pair<ItemIdType, DistType>& pair, 
    const string& namenodeKey, const string& portKey, const string& outputPathKey) {
    writeHDFSTriplet(queryId, pair.first, pair.second, namenodeKey, portKey, outputPathKey);
}

template<typename ItemIdType, typename DistType>
void writeHDFSTriplet(
    const ItemIdType& queryId, const std::pair<DistType, ItemIdType>& pair, 
    const string& namenodeKey, const string& portKey, const string& outputPathKey) {
    writeHDFSTriplet(queryId, pair.second, pair.first, namenodeKey, portKey, outputPathKey);
}
}
}
