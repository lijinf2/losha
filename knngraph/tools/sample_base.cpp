#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>

#include "lshbox/utils.h"
using namespace std;

void alertOutputFormatError() {
    cout << "output format error!" << std::endl;
    cout << "we currently only support outputs in fvecs or idfvecs" << std::endl;
    cout << "please provide a output path that ends with fvecs or idfvecs such as data_query.fvecs" << std::endl;
}

void sampleIdFVECS(const char* base_file, int num_samples, const char* output_file) {
    ifstream fin(base_file, ios::binary | ios::ate);
    if (!fin) {
        cout << "cannot open file " << base_file << endl;
        assert(false);
    }

    unsigned fileSize = fin.tellg();
    assert(fileSize != 0);

    fin.seekg(0, fin.beg);
    int id;
    int dimension;
    fin.read((char*)(&id), sizeof(int));
    fin.read((char*)(&dimension), sizeof(int));

    unsigned bytesPerRecord = dimension * sizeof(float) + 8;
    assert(fileSize % bytesPerRecord == 0);
    int cardinality = fileSize / bytesPerRecord;
    auto selected = sampleRand(cardinality, num_samples);

    fin.seekg(0, fin.beg);
    char* buffer = new char[bytesPerRecord];

    ofstream fout(output_file, ios::binary);
    if (!fout) {
        cout << "cannot open file " << output_file << endl;
        assert(false);
    }

    unsigned index = 0;
    while (fin.read((char*)buffer, bytesPerRecord)) {
        if (selected.find(index) != selected.end()) {
            cout << index << " th item selected" << endl;
            fout.write((char*)buffer, bytesPerRecord);
        }
        index++;
    }

    cout << "sampled base vectors are written into " << output_file << endl;
    fout.close();
    fin.close();
}

int main(int argc, char ** argv) {
    if (argc != 4) {
        cout << "Usage: ./sample_queries input_base_fvecs num_samples output_file" << endl;
    }

    const char* baseFile = argv[1];
    int numSamples = stoi(argv[2]);
    string outputFile = argv[3];
    if (outputFile.size() < 7 
        || (outputFile.substr(outputFile.size() - 5, 5) != "fvecs" && outputFile.substr(outputFile.size() - 7, 7) != "idfvecs")) {
        alertOutputFormatError();
    }
    sampleIdFVECS(baseFile, numSamples, outputFile.c_str());
    return 0;
}
