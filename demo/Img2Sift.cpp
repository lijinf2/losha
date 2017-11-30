/*
* @file SURF_FlannMatcher
* @brief SURF detector + descriptor + FLANN Matcher
* @author A. Huaman
* compile this file with : g++ -std=c++11 Img2Sift.cpp -o Img2Sift `pkg-config opencv --cflags --libs`
*/
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;
void readme();

int main(int argc, char **argv) {
  if (argc < 3) {
    readme();
    return -1;
  }

  // transfer single image
  // ./I2S2B <image> <OutputName>
  if (argc == 3) {
    Mat img = imread(argv[1], IMREAD_GRAYSCALE);  // read image
    const char *name = argv[2];
    Mat final_img;

    if (!img.data) {
      std::cout << " --(!) Error reading images " << std::endl;
      return -1;
    }

    cv::Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();  // creat SIFT object

    //-- Step 1: Detect the keypoints:
    std::vector<KeyPoint> keypoints;  // init keypoint datatype
    f2d->detect(img, keypoints);      // detect image keypoint

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptors;  // save keypoint to descriptor
    f2d->compute(img, keypoints, descriptors);

    //-- descriptor (Mat) to vector vector float
    // http://stackoverflow.com/questions/9790124/converting-a-row-of-cvmat-to-stdvector//
    std::vector<std::vector<float> > SIFTdata;
    for (int i = 0; i < descriptors.rows; i++) {
      const float *p = descriptors.ptr<float>(i);
      std::vector<float> row(p, p + descriptors.cols);
      SIFTdata.push_back(row);
    }
cout << SIFTdata.size()<<endl;
    ofstream out_file;
    out_file.open(name, ios_base::binary);
    for (int i = 0; i < SIFTdata.size(); i++) {
      out_file.write((const char *)&i, sizeof(i));
      for (int j = 0; j < SIFTdata[i].size(); j++) {
        out_file.write((const char *)&SIFTdata[i][j], sizeof(SIFTdata[i][j]));
      }
    }

    return 0;
  }

  // transfer images in folder
  // ./I2S2B <number> <image> <OutputName>
  if (argc == 4) {
    int totaldata = 0;
    int numberOfFile = stoi(argv[1]);
    const char *outfilename = argv[3];
    if (numberOfFile > 1000) {
      cout << "Currently support only 1000 images.";
      return -1;
    }
    cout << "Total: " << numberOfFile << " file(s) to be generated" << endl;

    // Reading directory
    DIR *dir;
    struct dirent *ent;

    std::vector<String> filenames;

    int counter = 0;
    if ((dir = opendir(argv[2])) != NULL) {
      while ((ent = readdir(dir)) != NULL && counter < numberOfFile + 2) {
        if (counter == 0 || counter == 1) {
          counter++;
          continue;
        }
        filenames.push_back(ent->d_name);
        counter++;
      }
      closedir(dir);
    } else {
      perror("Error reading directory: ");
      return -1;
    }

    /*for (auto n : filenames){
    cout << n << endl;
    }
    cout<< filenames.size() <<endl;*/

    // Preparing outfile//
    ofstream out_file;
    out_file.open(outfilename, ios_base::binary);
    counter = 0;
    for (auto file : filenames) {
      std::string f(argv[2]);  // read in filename

      std::stringstream ss;  // create fileId string
      ss << std::setw(3) << std::setfill('0')
         << counter;  // NOTE: This should be modified by number of inputs,
                      // currently support 000-999
      std::string fid = ss.str();

      fid = f + "/" + fid + ".jpg";  // alternate the file and path
      f = f + "/" + file;

      cout << "Image: " << f << " generating...";
      Mat img = imread(f, IMREAD_GRAYSCALE);  // read image

      int result = rename(f.c_str(), fid.c_str());

      Mat final_img;

      if (!img.data) {
        std::cout << " --(!) Error reading images " << std::endl;
        return -1;
      }

      cv::Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();  // creat SIFT
                                                             // object

      //-- Step 1: Detect the keypoints:
      std::vector<KeyPoint> keypoints;  // init keypoint datatype
      f2d->detect(img, keypoints);      // detect image keypoint

      //-- Step 2: Calculate descriptors (feature vectors)
      Mat descriptors;  // save keypoint to descriptor
      f2d->compute(img, keypoints, descriptors);

      //-- descriptor (Mat) to vector vector float
      // http://stackoverflow.com/questions/9790124/converting-a-row-of-cvmat-to-stdvector//
      std::vector<std::vector<float> > SIFTdata;
      for (int i = 0; i < descriptors.rows; i++) {
        const float *p = descriptors.ptr<float>(i);
        std::vector<float> row(p, p + descriptors.cols);
        SIFTdata.push_back(row);
        totaldata++;
      }

      for (int i = 0; i < SIFTdata.size(); i++) {
        int id = i * 1000 + counter;
        out_file.write((const char *)&id, sizeof(id));
        for (int j = 0; j < SIFTdata[i].size(); j++) {
          out_file.write((const char *)&SIFTdata[i][j], sizeof(SIFTdata[i][j]));
        }
      }
      counter++;
      cout << "done!" << endl;
    }
    
    cout << "Number of SIFT items: " << totaldata << endl;
    return 0;
  }
}

void readme() {
  std::cout << " Usage: ./Img2SIFT <img> <output_name>" << std::endl;
  std::cout << " Usage: ./Img2SIFT <number> <dir> <output_name>" << std::endl;
}

/* Img2SIFT translates an image to grayscale and extract it's key feature as
*SIFT descriptor
** Input: an image file
** Output: cout the result as vector< vector<float> >
** Each inner vector compose of id + 128 feature in datatype float.
** hidden function: mark the output on image with imread display
*/
