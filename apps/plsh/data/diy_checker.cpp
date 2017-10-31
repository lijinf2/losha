#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(){

    std::ifstream itemFile("diy_item.txt");
    std::ifstream queryFile("diy_query.txt");

    std::vector < std::vector < std::pair<int, float> > > items;
    std::vector < std::vector < std::pair<int, float> > > querys;

    if ( itemFile.is_open() )
    {
        std::string line;
        while ( getline(itemFile, line) )
        {
            //std::cout << line << "\n";

            boost::char_separator<char> sep(" \t");
            boost::tokenizer< boost::char_separator<char> > tokens(line, sep);
            
            bool setId = false;
            int itemId;
            std::vector< std::pair<int, float> > item;

            for ( auto &token : tokens )
            {
                if ( !setId ) {
                    itemId = std::stoi(token);
                    setId = true;
                }
                else {
                    int splitter = token.find(":");
                    int index = std::stoi( token.substr(0, splitter) );
                    float value = std::stof( token.substr(splitter + 1) );
                    item.push_back( std::make_pair( index, value ) );
                }
            }

            std::string temp = std::to_string(itemId) + " ";
            for (int i = 0; i < item.size(); ++i)
            {
                temp += std::to_string( item[i].first ) + ":" + std::to_string( item[i].second );
            }
            std::cout << temp << "\n";

            items.push_back(item);
        }
        itemFile.close();
    }

    return 1; 

}
