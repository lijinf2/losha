#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <boost/tokenizer.hpp>
#include <algorithm>

using namespace std;

int main(int argc, char** argv)
{
    int numOfThreads = atoi(argv[1]);
    
    map<int, vector< pair<int, float> > > groundTruth;
    map<int, vector< pair<int, float> > > appResult;

    // Read in groundtruth file
    string line;
    string fileName = argv[3];
    ifstream lshboxFile(fileName);
    if ( lshboxFile.is_open() )
    {
        getline(lshboxFile, line); // The first line indicating that we have 1000 queries, each has top-10 nearest items.
        while ( getline(lshboxFile, line) )
        {
            int queryId;
            int itemId;
            float distance;
            vector< pair<int, float> > oneResult;
           
            boost::char_separator<char> sep("\t");
            boost::tokenizer< boost::char_separator<char> > tok(line, sep);
            boost::tokenizer< boost::char_separator<char> >::iterator it = tok.begin(); 
            queryId = stoi(*it++);
            while ( it != tok.end() )
            {
                itemId = stoi(*it++);
                if ( it == tok.end() )
                {
                    break;
                }
                distance = stof(*it++);
                oneResult.push_back( make_pair(itemId, distance) );
            }
            groundTruth[queryId] = oneResult;    
        }
        lshboxFile.close();
    }
    else
    {
        cout << "Unable to open lshbox file!\n";
    }

    // DEBUG ONLY
    //cout << "groundtruth.size(): " << groundTruth.size() << "\n";
    //
    //for (const auto &query : groundTruth)
    //{
    //    cout << query.first << "\n";
    //    for (const auto &item : query.second)
    //    {
    //        cout << item.first << "\t";
    //        cout << item.second << "\n";
    //    }
    //    cout << "\n";
    //    return 0;
    //}
    //return 0;
    
    // Find largest
    //float largest_distance = 0; 
    //for (const auto &query : groundTruth)
    //{
    //    for (const auto &item : query.second)
    //    {
    //        if ( largest_distance < item.second )
    //        {
    //            largest_distance = item.second;
    //        }
    //    }
    //}
    //cout << "Largest distance: " << largest_distance << "\n";

    // Read in simhash result
    for ( int threadIter = 0; threadIter < numOfThreads; ++threadIter)
    {
        line = "";
        fileName = argv[2] + to_string(threadIter);
        ifstream resultFile(fileName);
        if ( resultFile.is_open() )
        {
            while ( getline(resultFile, line) )
            {            
                int queryId;
                int itemId;
                float distance;
                
                boost::char_separator<char> sep(" ");
                boost::tokenizer< boost::char_separator<char> > tok(line, sep);
                boost::tokenizer< boost::char_separator<char> >::iterator it = tok.begin(); 
                queryId = stoi(*it++);
                itemId = stoi(*it++);
                distance = stof(*it++);
                
                map<int, vector< pair<int, float> > >::iterator queryExist;
                queryExist = appResult.find(queryId);
                if (queryExist == appResult.end() )
                {
                    vector< pair<int, float> > oneResult;
                    oneResult.push_back( make_pair(itemId, distance) );
                    appResult[queryId] = oneResult;
                }
                else
                {
                    queryExist->second.push_back( make_pair(itemId, distance) );
                }
            }
            resultFile.close();
        }
        else
        {
            cout << "Unable to open Input Result!\n";
        }
    }

    // Sort appResult
    for (auto &query : appResult)
    {
        sort( query.second.begin(), query.second.end(), []( pair<int, float> &left, pair<int, float> &right )
            {
                return left.second < right.second;  
            });
    }

    // Remove extra item in appResult
    for (auto &query : appResult)
    {
        if ( query.second.size() > 10 )
        {
            query.second.erase( query.second.begin() + 10, query.second.end() );
        }
        //else
        //{
        //    cout << "Jesus! Not even find enough neighbors!\n";
        //    cout << "queryId: " << query.first << "\tsize(): " << query.second.size() << "\n";
        //}
    }

    // DEBUG ONLY
    //for (const auto &query : appResult)
    //{
    //    cout << "queryId: " << query.first << "\t size(): " << query.second.size() << "\n";
    //    for (const auto &item : query.second)
    //    {
    //        cout << item.first << "\t";
    //        cout << item.second << "\t";
    //    }
    //    cout << "\n\n"; 
    //}
    //cout << "appResult.size(): " << appResult.size() << "\n";

    // Calculate Error Ratio
    int numberOfItem = 0;
    float errorSum = 0;
    for ( const auto &query : appResult )
    {
        // Check app result size() is right
        if ( query.second.size() > 10 )
        {
            cout << "Wow! item.second.size() is more than 10! Erase part may have problems.\n";
            return 0;
        }
        int queryId = query.first;
        
        //// DEBUG ONLY: Check queryId exist in groundTruth
        //map<int, vector< pair<int, float> > >::iterator queryExist;
        //queryExist = groundTruth.find(queryId);
        //if (queryExist == groundTruth.end() )
        //{
        //    cout << "Error! Some queryId is not found in groundTruth!\n";
        //    return 0; 
        //}
        //// DEBUG ONLY:  
        //if ( query.second.size() > groundTruth.at(queryId).size() )
        //{
        //    cout << "Error! Index will be out of bound!\n";  
        //    return 0; 
        //}

        for ( int i = 0; i < query.second.size(); ++i )
        {
            pair<int, float> trueItem = groundTruth.at(queryId).at(i);
            pair<int, float> ourItem = query.second.at(i);
            float trueDistance = trueItem.second;
            float ourDistance = ourItem.second;
            float errorInThisPosition;
            
            if ( trueDistance == 0 || ourDistance == 0 )
            {
                // Some items are the same as queries
                errorInThisPosition = 1;
            }
            else if ( ourDistance < trueDistance )
            {
                // In this implementation, as simhash did not find enough items candidate, after sorting them, in the same index position, simhash result may have a smaller distance than the groundtruth result.
                errorInThisPosition = 1;
            }
            else
            {
                errorInThisPosition = ourDistance / trueDistance;
            }
            // In this implementation, error could be less than one.
            //if ( errorInThisPosition < 1)
            //{
            //    cout << "Error! It should not be bigger than one!\n";
            //    return 0;
            //}
            //cout << "errorInThisPosition: " << errorInThisPosition << "\n";
            errorSum += errorInThisPosition;
            numberOfItem++;
        }
    }
    float errorRatio = errorSum / numberOfItem;

    string outputName = argv[4];
    ofstream outputFile;
    outputFile.open( outputName );
    if ( outputFile.is_open() )
    {
        outputFile << "Error Ratio: " << errorRatio << "\n";
        outputFile << "Error Sum: " << errorSum << "\n";
        outputFile << "Number Of Item: " << numberOfItem << "\n";    
    }
    else
    {
        cout << "Error. Cannot open output file. \n";
        return 0;
    }
    outputFile.close();

    // Problem:
    // Current simhash losha implementation did not find enough nearest neighbors. Some query may not find enough 10 return items.

    return 0;
}
