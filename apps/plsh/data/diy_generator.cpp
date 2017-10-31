#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <algorithm>

int main() {

    std::ofstream itemFile("diy_item.txt");
    std::ofstream queryFile("diy_query.txt");

    int itemNum             = 500;
    int queryNum            = 50;
    int totalDimensionNum   = 20;
    int maxDimensionNum     = 10;

    srand(time(NULL));

    // generate itemFile

    for (int i = 0; i < itemNum; ++i){
        
        std::vector<int> dimList;
        int numOfDimension = rand() % maxDimensionNum + 1;
        float dimValue = 1.0 / sqrt(numOfDimension);
        std::string item = std::to_string(i) + " ";

        for (int j = 0; j < numOfDimension; ++j){
            bool keepGoing = true;
            int temp;
            while( keepGoing ){
                keepGoing = false;
                temp = rand() % totalDimensionNum;
                for (int k = 0; k < dimList.size(); ++k){
                    if (dimList[k] == temp){
                        keepGoing = true;
                        break;
                    }
                }
            }
            dimList.push_back(temp);
        }

        sort(dimList.begin(), dimList.end());
        for (int j = 0; j < dimList.size(); ++j){
            item += std::to_string(dimList[j]) + ":" + std::to_string(dimValue) + " ";
        }

        //std::cout << item << std::endl;
        itemFile << item << std::endl;

    }
    
    // generate queryFile

    for (int i = 0; i < queryNum; ++i){
        
        std::vector<int> dimList;
        int numOfDimension = rand() % maxDimensionNum + 1;
        float dimValue = 1.0 / sqrt(numOfDimension);
        std::string item = std::to_string(i) + " ";

        for (int j = 0; j < numOfDimension; ++j){
            bool keepGoing = true;
            int temp;
            while( keepGoing ){
                keepGoing = false;
                temp = rand() % totalDimensionNum;
                for (int k = 0; k < dimList.size(); ++k){
                    if (dimList[k] == temp){
                        keepGoing = true;
                        break;
                    }
                }
            }
            dimList.push_back(temp);
        }

        sort(dimList.begin(), dimList.end());
        for (int j = 0; j < dimList.size(); ++j){
            item += std::to_string(dimList[j]) + ":" + std::to_string(dimValue) + " ";
        }

        //std::cout << item << std::endl;
        queryFile << item << std::endl;

    }

    itemFile.close();
    queryFile.close();

    return 0;
}
