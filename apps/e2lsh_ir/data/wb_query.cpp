#include <fstream>
#include <iostream>

int main() {

    std::ofstream outfile("query_ssb.bin", std::ofstream::binary);

    int x = 0;
    int y = 1;

    for (int i = 0; i < 2; i++){
        
        outfile.write(reinterpret_cast<const char *>(&i), sizeof(i));
        std::cout << i << " ";
        for (int j = 0; j < 5; j++){
        
            if ( i == 1 && j == 0 ){
                outfile.write(reinterpret_cast<const char *>(&y), sizeof(y));
                std::cout << y << " ";
            }else{
                outfile.write(reinterpret_cast<const char *>(&x), sizeof(x));
                std::cout << x << " ";
            }
        }
        std::cout << "\n";

    }

}
