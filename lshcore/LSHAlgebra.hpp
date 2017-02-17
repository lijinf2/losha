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

#pragma once
#include <cmath>
#include <utility>
#include <vector>

// // #include "LSHUtils.hpp"
//
// /*
//  * Declaration
//  * */
// // return differences between each projection and corresponding hash value
// // distance not difference
// std::vector< std::vector<float> > E2LSHCalDistance(
//         std::vector< std::vector<float> >& projections,
//         std::vector< std::vector<int> >& hashValues);
//
// /* vector operation */
// template<typename T>
// float getL2Norm(const std::vector<T>& vec);
//
// template<typename T>
// float getL2Norm(const std::vector< std::pair<int, T> >&  vec);
//
// /*
//  * Implementation
//  * */
//
// // region: Vector operations
//
// // for denseVector
// template<typename T>
// float getL2Norm(const std::vector<T>& vec) {
//     float sum = 0;
//     for (auto& v : vec) {
//         sum += v * v;
//     }
//     return sqrt(sum);
// }
//
// template<typename T>
// float getL2Norm(const std::vector< std::pair<int, T> >& vec) {
//     float sum = 0;
//     for (auto& v : vec) {
//         sum += v.second * v.second;
//     }
//     return sqrt(sum);
// }
// std::vector< std::vector<float> > E2LSHCalDistance(
//         std::vector< std::vector<float> >& projections,
//         std::vector< std::vector<int> >& hashValues) {
//     // checking
//     int bands = projections.size();
//     int rows = projections.back().size();
//     assert(bands  == hashValues.size());
//     assert(rows  == hashValues.back().size());
//
//
//     std::vector< std::vector<float> > difference;
//     difference.resize(bands);
//
//     for (int i = 0; i < bands; ++i) {
//         difference[i].resize(rows);
//     }
//
//     for (int i = 0; i < bands; ++i) {
//         assert(projections[i].size() == rows);
//         assert(hashValues[i].size() == rows);
//         for (int j = 0; j < rows; ++j) {
//             difference[i][j] = projections[i][j] - hashValues[i][j] * LSHContext::getW();
//         }
//     }
//
//     return difference;
// }
//
