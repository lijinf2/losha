#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "boost/tokenizer.hpp"

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include <eigen/Eigen/Dense>
#include <eigen/Eigen/SVD>

using Eigen::MatrixXf;

// All data points are assumed to be of type float, except IDs
// For debugging, assumed 3 calculators/machiens only
// Calculator id starts from 0

//MatWrap wraps an Eigen matrix into a vector

std::vector<float> MatWrap(MatrixXf Mat) {
    int r = Mat.rows();
    int c = Mat.cols();
    std::vector<float> content;
    //std::cout << " r: " << r << ", " << "c: " << c << ", " << "Mat: " << Mat;
    for (int i = 0; i < r ; i++){
        for (int j = 0; j < c; j++){
            content.push_back(Mat(i, j));
        }
    }
    content.shrink_to_fit();
    return content;
}

//new MatWrap
std::vector<bool> MatWrapB(MatrixXf Mat) {
    int r = Mat.rows();
    int c = Mat.cols();
    std::vector<bool> content;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            content.push_back((Mat(i, j) > 0));	
        }
    }
    content.shrink_to_fit();
    return content;
}

//unWrapMat unwraps a vector into Eigen matrix, r is number of rows, c is number of columns
MatrixXf unWrapMat(std::vector<float> content, int r, int c) {
    MatrixXf X(r, c);
    for (int i = 0; i < r; i++){
        for (int j = 0; j < c; j++){
            X(i, j) = content[c*i+j];
        }
    }
    return X;
}

MatrixXf unWrapMatB(std::vector<bool> content, int r, int c) {
    MatrixXf X(r, c);
    for (int i = 0; i < r; i++){
        for (int j = 0; j < c; j++){
            if (content[c*i+j]) {
                X(i, j) = 1;
            }
            else {
                X(i, j) = -1;
            }
        }
    }
    return X;
}

MatrixXf unWrapMatX(std::vector<unsigned char> content, int r, int c) {
    MatrixXf X(r, c);
    for (int i = 0; i < r; i++){
        for (int j = 0; j < c; j++){
            X(i, j) = (float)(content[c*i+j]); 
        }
    }
    return X;
}

//Function for quantization B = sgn(C.transpose()*X) !!Problem may exist with this function
MatrixXf quantization(MatrixXf C, MatrixXf X){
    MatrixXf B = C.transpose()*X;
    int r = B.rows();
    int c = B.cols();
    for (int i = 0; i < r ; i++){
        for (int j = 0; j < c; j++){
            if (B(i, j) >= 0){
                B(i, j) = 1;
            }
            else {
                B(i, j) = -1;
            }
        }
    }
    return B;
}

class Calculator { // a DisH node (virtual node)
    public:
        typedef int KeyT;
        Calculator() = default;
        explicit Calculator(const KeyT cid) : calculator_id(cid) {} 
        const KeyT id() const { return calculator_id; }

        //X the chunk of data assigned, C transformation matrix, Cin1 and Cin2 are updated Cs received from neighbors, B the codebook, Lambda the Lagrangian
        //n stands for number of data points in each calculator, INSTEAD of in total
        //d for dimension
        //r for code length
        KeyT calculator_id;
        std::vector<unsigned char> X;
        std::vector<int> vectorList;
        std::vector<bool> B;

        std::vector<float> C;
        std::vector<float> Cin1;
        std::vector<float> Cin2;
        std::vector<float> Lambda;
        int n, r, d;

        friend husky::BinStream& operator<<(husky::BinStream& stream, const Calculator& Cal) {
            stream << Cal.vectorList << Cal.calculator_id << Cal.n << Cal.r << Cal.d << Cal.X << Cal.C << Cal.Cin1 << Cal.Cin2 << Cal.B << Cal.Lambda;
            return stream;
        }
        friend husky::BinStream& operator>>(husky::BinStream& stream, Calculator& Cal) {
            stream >> Cal.vectorList >> Cal.calculator_id >> Cal.n >> Cal.r >> Cal.d >> Cal.X >> Cal.C >> Cal.Cin1 >> Cal.Cin2 >> Cal.B >> Cal.Lambda;
            return stream;
        }	
};

void hashingDD() {
    int numVectors = stoi(husky::Context::get_param("size"));
    int dimension = stoi(husky::Context::get_param("dimension"));
    int codeLength = stoi(husky::Context::get_param("codeLength"));
    int numNodes = stoi(husky::Context::get_param("numberOfNodes"));
    std::string inputPath = husky::Context::get_param("hashingDDinput");

    // This algorithm requirs that n|numVectors
    assert(numVectors % numNodes == 0);
    int n = numVectors/numNodes;
    int chunk_size = (8 + dimension)*n;
    if (husky::Context::get_global_tid() == 0) {
        husky::LOG_I << "parameter numVectors = " << numVectors << std::endl;
        husky::LOG_I << "parameter dimension = " << dimension << std::endl;
        husky::LOG_I << "parameter codeLength = " << codeLength << std::endl;
        husky::LOG_I << "parameter numNods = " << numNodes << std::endl;
        husky::LOG_I << "parameter n (i.e. vectors per calculator) = " << n << std::endl;
        husky::LOG_I << "parameter chunksize (i.e. (8 + 128) * n) = " << chunk_size << std::endl;
        husky::LOG_I << "parameter inputPath = " << inputPath << std::endl;
    }

    auto& infmt = husky::io::InputFormatStore::create_chunk_inputformat(chunk_size);
    infmt.set_input(inputPath);

    // Initialize C so that transpose(C)*C = I_r, initialize Lambda to be 0.6
    MatrixXf Cinit(dimension, codeLength);
    MatrixXf Lambdainit(dimension, codeLength);
    for (int i = 0; i < dimension; i++){
        for (int j = 0; j < codeLength; j++){
            if (i == j){
                Cinit(i, j) = 1;
                Lambdainit(i, j) = 0.6;
            }
            else {
                Cinit(i, j) = 0;
                Lambdainit(i, j) = 0;
            }
        }
    }

    std::vector<float> Cwrapped = MatWrap(Cinit);
    std::vector<float> Lambdawrapped = MatWrap(Lambdainit);

    auto& calculatorList = husky::ObjListStore::create_objlist<Calculator>();

    auto parse_mat = [&calculatorList, &Cwrapped, &Cinit, &Lambdawrapped, &dimension, &n](boost::string_ref& chunk){
        //parse X and send to each node
        if (chunk.size() == 0)
            return;

        int id;
        memcpy(&id, &chunk[0], 4);
        id = id/n;
        Calculator cal(id);
        cal.X.reserve(n * (dimension + 8));
        for (int i = 0; i < n*(dimension +8); i++){
            //Convert binaries to floats
            if (i%(dimension+8) == 0){
                int vectorid;
                memcpy(&vectorid, &chunk[i], 4);
                cal.vectorList.push_back(vectorid);
            }
            else if (i%(dimension+8) >= 8){
                unsigned char duchar = chunk[i];
                //float db = (float)(dbint);
                //need to read X as unsigned char
                cal.X.push_back(duchar);
            }
        }
        cal.X.shrink_to_fit();
        cal.n = n;
        calculatorList.add_object(std::move(cal));
        if(husky::Context::get_global_tid() == 0){
            husky::LOG_I << "calculator of id " << id << "created." << std::endl;
        }
    };

    husky::load(infmt, parse_mat);
    husky::globalize(calculatorList);

    auto& ch = husky::ChannelStore::create_push_channel<std::vector<float>>(calculatorList, calculatorList);

    if(husky::Context::get_global_tid() == 0){
        husky::LOG_I << "start initialization" << std::endl;
    }

    // prepare the initial B and Lambda
    husky::list_execute(calculatorList, [&dimension, &codeLength, &Cwrapped, &Lambdawrapped, &Cinit](Calculator& cal){
        cal.X.shrink_to_fit();
        MatrixXf Xnew = unWrapMatX(cal.X, dimension, cal.n);
        cal.C = Cwrapped;
        cal.Cin1 = Cwrapped;
        cal.Cin2 = Cwrapped;
        MatrixXf Binit = quantization(Cinit, Xnew);
        //need to wrap B as vector of bool
        std::vector<bool> Bwrapped = MatWrapB(Binit);
        cal.B = Bwrapped;
        cal.Lambda = Lambdawrapped;
    });

    if(husky::Context::get_global_tid() == 0){
        husky::LOG_I << "finished initialization" << std::endl;
    }

    // Need to tune this parameter 3 to larger when needed
    for (int i = 0; i < 20; i++){

        if(husky::Context::get_global_tid() == 0){
            husky::LOG_I << "start iteration " << i << std::endl;
        }

        husky::list_execute(calculatorList, [&ch, &dimension, &n, &codeLength, &numNodes](Calculator& cal){
            //need to unwrap X as vector of unsigned char
            MatrixXf Xnew = unWrapMatX(cal.X, dimension, n);
            //need to unwrap B as vector of bool
            MatrixXf Bnew = unWrapMatB(cal.B, codeLength, n);
            MatrixXf Cin1new = unWrapMat(cal.Cin1, dimension, codeLength);
            MatrixXf Cin2new = unWrapMat(cal.Cin2, dimension, codeLength);
            MatrixXf Lambdanew = unWrapMat(cal.Lambda, dimension, codeLength);
            //Compute S, using ro = 0.1
            MatrixXf S = 2*Bnew*Xnew.transpose() - Lambdanew.transpose() + 0.1*(Cin1new.transpose() + Cin2new.transpose());
            //SVD on S
            Eigen::JacobiSVD<MatrixXf> Ssvd(S, Eigen::DecompositionOptions::ComputeFullU | Eigen::DecompositionOptions::ComputeThinV);
            MatrixXf H = Ssvd.matrixV();
            MatrixXf G = Ssvd.matrixU();
            MatrixXf Cupdated = H*G.transpose();
            std::vector<float> Cupdatedwrapped = MatWrap(Cupdated);
            cal.C = Cupdatedwrapped;
            //Send C out
            int n1 = cal.id()+1;
            int n2 = cal.id()-1;
            n1 = (n1+numNodes)%numNodes;
            n2 = (n2+numNodes)%numNodes;
            ch.push(Cupdatedwrapped, n1);
            ch.push(Cupdatedwrapped, n2);
        });

        if(husky::Context::get_global_tid() == 0){
            husky::LOG_I << "finish the first list_execute" << std::endl;
        }

        husky::list_execute(calculatorList, [&ch, &dimension, &codeLength](Calculator& cal){
            std::vector<std::vector<float>> received = ch.get(cal);
            if (received.size() > 1){
            cal.Cin1 = received[0];
            cal.Cin2 = received[1];
            MatrixXf Cnew = unWrapMat(cal.C, dimension, codeLength);
            MatrixXf Cin1new = unWrapMat(cal.Cin1, dimension, codeLength);
            MatrixXf Cin2new = unWrapMat(cal.Cin2, dimension, codeLength);
            MatrixXf Lambdaold = unWrapMat(cal.Lambda, dimension, codeLength);
            MatrixXf Lambdanew = Lambdaold + 2*0.1*(2*Cnew - Cin1new - Cin2new);
            cal.Lambda = MatWrap(Lambdanew);
            }
        });		

        if(husky::Context::get_global_tid() == 0){
            husky::LOG_I << i  << "th iteration has been finished." << std::endl;
        }
    }

    //Print out final results
    husky::list_execute(calculatorList, [&n, &dimension, &codeLength](Calculator& cal){
        //change cal.B to vector of bool
        MatrixXf Bnew = unWrapMatB(cal.B, codeLength, n);
        MatrixXf Cnew = unWrapMat(cal.C, dimension, codeLength);
        //change cal.X to vector of unsigned char
        MatrixXf Xnew = unWrapMatX(cal.X, dimension, n);
        MatrixXf Bupdate = quantization(Cnew, Xnew);
        //Each vector of id cal.vectorList[i] is hashed to Bupdate(:,i)
        std::string outputFileName("Output_");
        outputFileName += std::to_string(cal.id());
        outputFileName += ".txt";
        std::ofstream fo(outputFileName);
        for (int i = 0; i < n; i++){
            fo << std::to_string(cal.vectorList[i]) << " ";
            for (int j = 0; j < codeLength; j++){
                if (Bupdate(j, i) > 0)
                    fo << "1";
                else
                    fo << "0";
                fo << " ";
            }
            fo << "\n";
        }
        fo.close();
        husky::LOG_I << outputFileName << " has been finished." << std::endl;
    });
}

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("hashingDDinput");
    args.push_back("size");
    args.push_back("dimension");
    args.push_back("codeLength");
    args.push_back("numberOfNodes");
    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(hashingDD);
        return 0;
    }
}
