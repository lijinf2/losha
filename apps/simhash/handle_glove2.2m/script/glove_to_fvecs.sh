mkdir output

g++ -O3 glove_to_fvecs.cpp -o ./output/glove_to_fvecs.bin

#./output/glove_to_fvecs.bin [PATH_TO_GLOVE_DATASET] ./output/glove2.2m_base.fvecs
./output/glove_to_fvecs.bin ./raw_data/glove.840B.300d.txt ./output/glove2.2m_base.fvecs
