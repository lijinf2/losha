/*
 * Copyright 2016 husky Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "boost/functional/hash.hpp"
#include "base/log.hpp"
#include "core/engine.hpp"
#include "densevector.hpp"

namespace husky {
namespace losha {

void normalize(std::vector<float> &vec) {
    assert(vec.size() != 0);
    float sum = 0;
    for (auto& v : vec) {
        sum += v * v;
    }
    sum = sqrt(sum);
    assert(fabs(sum - 0) > 0.0000000001);
    for (int i = 0; i < vec.size(); ++i) {
        vec[i] /= sum;
    }
}

void normalize(std::vector<std::pair<int, float> > &vec) {
    assert(vec.size() != 0);
    float sum = 0;
    for (auto& v : vec) {
        sum += v.second * v.second;
    }
    sum = sqrt(sum);
    assert(fabs(sum - 0) > 0.0000000001);
    for (int i = 0; i < vec.size(); ++i) {
        vec[i].second /= sum;
    }
}

std::pair<int, float> lshStofPair(const std::string& pairStr) {
    int splitter = pairStr.find(':');
    int index = std::stoi(pairStr.substr(0, splitter));
    float value = std::stof(pairStr.substr(splitter+1));
    return std::pair<int, float>(index, value);
}

} // namespace losha
} // namespace husky
// /*
//  * declaration
//  * */
//
// namespace std {
//
// std::string to_string(const std::string& str) {
//     return str;
// }
// template<typename firstT, typename secondT>
// std::string to_string(const std::pair<firstT, secondT>& p);
//
// template<typename T>
// std::string to_string(const std::vector<T>& v);
//
// template<typename T>
// std::string to_string(const std::vector< std::vector<T> >& matrix);
//
// template<typename ItemIdType, typename ItemElementType>
// std::string to_string(const DenseVector<ItemIdType, ItemElementType>& p);
//
// // hash for vectors
// template<typename T>
// class hash< std::vector<T> > {
//         public:
//             size_t operator()(const std::vector<T> vec) const;
// };
//
// }  // namespace std
//
// // report contents in obj lists, require obj implement toString()
// template<typename ObjType>
// void reportList(husky::ObjList<ObjType> list);
//
// // user defined parameters
//
// inline bool getParamExistence(const std::string& str) {
//     return ( husky::Context::get_param(str) != "" );
// }
//
// inline std::string getParam(const std::string& str) {
//     std::string log_str = "cannot find parameter of key " + str;
//     ASSERT_MSG(husky::Context::get_param(str) != "", log_str.c_str());
//     return husky::Context::get_param(str);
// }
//
// inline std::string getParamStr(const std::string& str) {
//     std::string log_str = "cannot find parameter of key " + str;
//     ASSERT_MSG(husky::Context::get_param(str) != "", log_str.c_str());
//     return husky::Context::get_param(str);
// }
//
// inline int getParamInt(const std::string& str) {
//     std::string log_str = "cannot find parameter of key " + str;
//     ASSERT_MSG(husky::Context::get_param(str) != "", log_str.c_str());
//     return std::stoi(husky::Context::get_param(str));
// }
//
// inline float getParamFloat(std::string str) {
//     std::string log_str = "cannot find parameter of key " + str;
//     ASSERT_MSG(husky::Context::get_param(str) != "", log_str.c_str());
//     return std::stof(husky::Context::get_param(str));
// }
//
// // inline int getBands() {
// //     return std::stoi(husky::Context::get_param("bands"));
// // }
// //
// // inline int getRows() {
// //     return std::stoi(husky::Context::get_param("rows"));
// // }
// //
// // inline int getDimension() {
// //     return std::stoi(husky::Context::get_param("dimension"));
// // }
// //
// // inline int getSeed() {
// //     return std::stoi(husky::Context::get_param("seed"));
// // }
// //
// // inline float getW() {
// //     return std::stof(husky::Context::get_param("W"));
// // }
// //
// // inline float getf() {
// //     return std::stof(husky::Context::get_param("f"));
// // }
// //
// // inline int getm() {
// //     return std::stoi(husky::Context::get_param("m"));
// // }
// //
// // inline std::string getOutputPath() {
// //     return husky::Context::get_param("outputPath");
// // }
//
//
// // extract itemId and dnese itemVector from string
// int lshStoi(const std::string& intString);
//
// float lshStof(const std::string& floatString);
//
// // extract itemId and sparse itmeVector from string
// std::pair<int, float> lshStofPair(const std::string& floatString);
//
// std::string lshStos(const std::string& strString);
//
// template<typename T>
// T parser(T (*Action)(std::string), std::string line);
//
// template<typename ItemIdType, typename ItemElementType>
// void setItemFromLine(
//         boost::string_ref& line,
//         ItemIdType& itemId,
//         ItemIdType (*idParser)(const std::string&),
//         std::vector<ItemElementType>& itemVector,
//         ItemElementType (*elementParser)(const std::string&));
//
// // comparator for std pair, ascending by first element
// template<typename FirstT, typename SecondT>
// static bool pairAscendComparator(std::pair<FirstT, SecondT> a,
//         std::pair<FirstT, SecondT> b);
//
// // comparator for std pair, descending by the first element
// template<typename FirstT, typename SecondT>
// static bool pairDescendComparator(std::pair<FirstT, SecondT> a,
//         std::pair<FirstT, SecondT> b);
//
//
// /*
//  * Implementation 
//  * */
//
// namespace std {
//
// template<typename firstT, typename secondT>
// std::string to_string(const std::pair<firstT, secondT> &p) {
//     return "(" + to_string(p.first) + "\t" + to_string(p.second) + ")";
// }
//
// template<typename T>
// std::string to_string(const std::vector<T> &v) {
//     std::string str = "<";
//     for (const T& e : v) {
//         str += std::to_string(e) + ", ";
//     }
//     str += ">";
//     return str;
// }
//
// template<typename T>
// std::string to_string(const std::vector< std::vector<T> >& matrix) {
//     std::string str = "\n\n[";
//     for (auto & vec : matrix) {
//         str += std::to_string(vec);
//         str += "\n";
//     }
//     str += "]\n\n";
//     return str;
// }
//
// template<typename ItemElementType>
// std::string to_string(const DenseVector<std::string, ItemElementType>& p) {
//     std::string result = "(id: " + p.getItemId() + "\titemVector: "
//         + std::to_string(p.getItemVector()) + ")\n";
//     return result;
// }
//
// template<typename ItemIdType, typename ItemElementType>
// std::string to_string(const DenseVector<ItemIdType, ItemElementType>& p) {
//     std::string result = "(id: " + std::to_string(p.getItemId()) + "\titemVector: "
//         + std::to_string(p.getItemVector()) + ")\n";
//     return result;
// }
//
// template<typename T>
// size_t hash< std::vector<T> >::operator()(const std::vector<T> vec) const {
//     size_t seed = 0;
//     for (auto& v : vec) {
//         boost::hash_combine(seed, v);
//     }
//     return seed;
// }
//
// }  // namespace std
//
// int lshStoi(const std::string& intString) {
//     return std::stoi(intString);
// }
//
// int lsh4BytesToInt(const std::string& str) {
//     int intNum;
//     memcpy(&intNum, &str[0], 4);
//     return intNum;
// }
//
// float lshStof(const std::string& floatString) {
//     return std::stof(floatString);
// }
//
// float lsh4BytesToFloat(const std::string& str) {
//     float fNum;
//     memcpy(&fNum, &str[0], 4);
//     return fNum;
// }
//
// std::string lshStos(const std::string& strString) {
//     return strString;
// }
//
// template<typename T>
// T parser(T (*Action)(std::string), std::string line) {
//         return Action(line);
// }
//
// template<typename ItemIdType, typename ItemElementType>
// void setItemFromLine(
//         boost::string_ref& line,
//         ItemIdType& itemId,
//         ItemIdType (*idParser)(const std::string&),
//         std::vector<ItemElementType>& itemVector,
//         ItemElementType (*elementParser)(const std::string&)) {
//     assert(line.size() != 0);
//
//     boost::char_separator<char> sep(" \t");
//     boost::tokenizer<boost::char_separator<char>> tok(line, sep);
//
//     // optimize to use dimension to avoid push back set lshStof
//     bool setId = false;
//     for (auto &w : tok) {
//         if (!setId) {
//             itemId = idParser(w);
//             setId = true;
//         } else {  // else itemVector.push_back(elementParser(w));
//             itemVector.push_back(elementParser(w));
//         }
//     }
// }
//
// template<typename FirstT, typename SecondT>
// static bool pairAscendComparator(std::pair<FirstT, SecondT> a,
//         std::pair<FirstT, SecondT> b) {
//     return a.second < b.second;
// }
//
// template<typename FirstT, typename SecondT>
// static bool pairDescendComparator(std::pair<FirstT, SecondT> a,
//         std::pair<FirstT, SecondT> b) {
//     return a.second > b.second;
// }
//
// // execution function
// template<typename ObjType>
// void reportList(husky::ObjList<ObjType> list) {
//     husky::list_execute(list,
//         [](ObjType obj) {
//         husky::base::log_msg(obj.toString());
//         });
// }


