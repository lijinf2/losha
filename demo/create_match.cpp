/*
* @file SURF_FlannMatcher
* @brief SURF detector + descriptor + FLANN Matcher
* @author A. Huaman
* compile this file with : g++ -std=c++11 create_match.cpp -o create_match `pkg-config opencv --cflags --libs`
*/

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
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

void twoImageDmatch(char* img1, char* img2, std::vector<DMatch> result) {
  Mat img_1 = imread(img1, IMREAD_GRAYSCALE);
  Mat img_2 = imread(img2, IMREAD_GRAYSCALE);
  Mat one = imread(img1, CV_LOAD_IMAGE_COLOR);
  Mat two = imread(img2, CV_LOAD_IMAGE_COLOR);
  // cout << "image1: " << img_1 << "\n";
  if (!img_1.data || !img_2.data) {
    std::cout << " --(!) Error reading images " << std::endl;
    return;
  }

  // Reconstruct the input descriptors
  cv::Ptr<Feature2D> detector =
      xfeatures2d::SIFT::create();  // creat SIFT object

  std::vector<KeyPoint> keypoints_1, keypoints_2;
  Mat descriptors_1, descriptors_2;
  detector->detectAndCompute(img_1, Mat(), keypoints_1, descriptors_1);
  detector->detectAndCompute(img_2, Mat(), keypoints_2, descriptors_2);

  // Read result from file to create matches
  

  // Draw matches and save as image
  Mat img_matches;
  drawMatches(one, keypoints_1, two, keypoints_2, result, img_matches,
              Scalar::all(-1), Scalar::all(-1), vector<char>(),
              DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

  imwrite("dir.jpg", img_matches);
  // for( int i = 0; i < (int)good_matches.size(); i++ )
  // { printf( "-- Good Match [%d] Keypoint 1: %d    -- Keypoint 2: %d    \n",
  // i, good_matches[i].queryIdx, good_matches[i].trainIdx ); }
  printf("number of good matches: %d\n", (int)result.size());
  waitKey(0);
  return;
}

void twoImage(char* img1, char* img2, char* result) {

  std::vector<DMatch> good_matches;  // matches
  ifstream infile;
  infile.open(result);

  int query, item;
  float distance;
  if (infile.is_open()) {
    while (!infile.eof()) {
      infile >> query >> item >> distance;
      DMatch pair = DMatch(query, item, distance);
      good_matches.push_back(pair);
      // cout << query << " " << item << " : " << distance <<endl;
    }
  }
  infile.close();

  twoImageDmatch(img1, img2, good_matches);
}

void dirsearch(char* img1, char* dir, char* result) {
  ifstream infile;
  infile.open(result);

  int qid, iidfid, fid, iid;
  float distance;

  // std::map<int, int> fidIndex;
  std::map<int, std::vector<DMatch>> fidMatch;

  if (infile.is_open()) {
    while (!infile.eof()) {
      infile >> qid >> iidfid >> distance;
      fid = iidfid % 1000;
      iid = iidfid / 1000;
      DMatch pair = DMatch(qid, iid, distance);
      fidMatch[fid].push_back(pair);
      // cout << qid << " " << fid << " " << iid << " : " << distance << endl;
    }
  }
  infile.close();

  int maxid = -1;
  int maxcount = -1;
  for (auto it = fidMatch.cbegin(); it != fidMatch.cend(); ++it) {
    int size = it->second.size();
    if (size > maxcount) {
      maxid = it->first;
      maxcount = size;
    }
  }

  // cout << maxid << " " << maxcount << endl;

  std::stringstream ss;  // create fileId string
  ss << std::setw(3) << std::setfill('0')
     << maxid;  // NOTE: This should be modified by number of inputs,
                // currently support 000-999
  std::string filename = ss.str();
  std::string dirname(dir);

  std::vector<DMatch> selection;
  for (int p=0; p<fidMatch[maxid].size(); p++){
      if (p%29 == 0) selection.push_back(fidMatch[maxid][p]);
  }

  filename = dirname + "/" + filename + ".jpg";  // alternate the file and path
  cout << filename << endl;
  twoImageDmatch(img1, const_cast<char*>(filename.c_str()), selection);
  return;
}

int main(int argc, char** argv) {
  if (!strcmp(argv[1], "-two")) {
    if (argc != 5) {
      std::cout << " Usage: -two <img1> <img2> <description>" << std::endl;
    }
    twoImage(argv[2], argv[3], argv[4]);
    return 0;
  }

  if (!strcmp(argv[1], "-dir")) {
    if (argc != 5) {
      std::cout << " Usage: -dir <img1> <dir> <description>" << std::endl;
    }
    dirsearch(argv[2], argv[3], argv[4]);
    return 0;
  }

  if (argc != 5) {
    readme();
    return -1;
  }

  return 0;
}

void readme() {
  std::cout << " Usage: -two <img1> <img2> <description>" << std::endl;
  std::cout << " Usage: -dir <img1> <dir> <description>" << std::endl;
}