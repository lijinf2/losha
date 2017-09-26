#include <fstream>
#include <iostream>

int main() {

    std::ofstream outfile("item_ssb.bin", std::ofstream::binary);

    int x = 0;
    int y = 1;

    for (int i = 2; i < 9; i++){
        
        outfile.write(reinterpret_cast<const char *>(&i), sizeof(i));
        std::cout << i << " ";
        for (int j = 0; j < 5; j++){
        
            if ( (i - j) == 3 ){
                outfile.write(reinterpret_cast<const char *>(&y), sizeof(y));
                std::cout << y << " ";
            }else if (i == 8){ 
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
