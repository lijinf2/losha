#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>

typedef unsigned char sift10[128*sizeof(float)];
using namespace std;

void readme();
void verify(string filename);

int main(int argc, char **argv){

    if (argc < 4){
        readme();
        return -1;
    }

    // setup
    string infile = argv[2];
    string outfile = argv[3];

    int id = 0;
    int number_of_vec = 0;
    int number_of_dim = 0;
    int size_of_item = 0;

    //setting
    if (!(strcmp(argv[1],"sift10"))){
        number_of_dim = 128;
        size_of_item = sizeof(float);
    }
    

    //main_process
    
    //file processing
    ofstream out_file;
    ifstream in_file;
    out_file.open(outfile, ios_base::binary);
    in_file.open(infile, ios_base::binary);

    //read-write
    while(!in_file.eof()){

        //setup
        int dummy;
        sift10 data;
        float d;

        //read
        in_file.read((char *)&dummy, sizeof(int));  // removes dummy
        in_file.read((char *)&data, sizeof(data));
        
        //write
        out_file.write((const char *)&id, sizeof(id));
        out_file.write((const char *)&data, sizeof(data));
    
        id++;
    }

    in_file.close();
    out_file.close();

//    verify(outfile); 
    return 0;
}

//bug in verify, but result is correct//
void verify(string filename){
    ifstream in_file;
    in_file.open(filename, ios_base::binary); 
    
    while(!in_file.eof()){
        int id;
        in_file.read((char *)&id, sizeof(int));
        cout << id << " ";
        for (int i = 0; i < 128; i++){
            float dd;
            in_file.read((char*)&dd, sizeof(dd));
            cout << dd << " ";
        }
        cout << endl << endl;
    }
}

void readme(void){
    cout << "\n";
    cout << "This is a utility tool to transform files from TEXMEX to Losha readable.\n";
    cout << "The final formate should be ID + list of data\n";
    cout << "Usage:\n";
    cout << "sift10 <input_name> <output_name>\n";
}
