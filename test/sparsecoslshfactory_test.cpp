#include "lshcore/sparse/sparsecoslshfactory.hpp"
#include "lshcore/sparse/apsparsecoslshfactory.hpp"
#include <iostream>
using namespace husky::losha;

int band = 10;
int row = 2;
int dimension = 10;
int main(){
    SparseCosLSHFactory<int, float> fac;
    fac.initialize(band, row, dimension);

    std::vector<std::pair<int , float>> itemVector;
    itemVector.push_back( std::make_pair(0, 0.1));
    itemVector.push_back( std::make_pair(1, 1.1));
    itemVector.push_back( std::make_pair(2, -0.1));
    itemVector.push_back( std::make_pair(3, -1.1));

    std::vector<bool> vec = fac.calSignaturesInBool(itemVector);
    for (int i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << " ";
    } 
    std::cout << std::endl;

    APSparseCosLSHFactory<int, float> apfac;
    apfac.initialize(band, row, dimension);

    std::vector<bool> vec2 = apfac.calSignaturesInBool(itemVector);
    for (int i = 0; i < vec2.size(); ++i) {
        std::cout << vec2[i] << " ";
    } 
    std::cout << std::endl;

}
