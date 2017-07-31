#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;

void readme();


static void show_window(const std::string& window_name, const cv::Mat& img)
{
	const float w = float(img.cols / 2);
	const float h = w * (float(img.cols) / float(img.rows));
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, (int)w, (int)h);
	//cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}


/** @function main */
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		readme(); return -1;
	}

	Mat img_object = imread(argv[1], IMREAD_GRAYSCALE);
	Mat img_scene = imread(argv[2], IMREAD_GRAYSCALE);
	float min_dist_tolerance = ((argc > 3) ? atof(argv[3]) : 3.0f);
	float max_distance_y = ((argc > 4) ? atof(argv[4]) : 25.0f);
	
	std::string output_file = ((argc > 4) ? argv[4] : "surf_result.tif");

	if (!img_object.data || !img_scene.data)
	{
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}

	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;

	Ptr<SURF> detector = SURF::create(minHessian);

	std::vector<KeyPoint> keypoints_object, keypoints_scene;

	detector->detect(img_object, keypoints_object);
	detector->detect(img_scene, keypoints_scene);

	// --Step 2: Calculate descriptors(feature vectors)
	Ptr<SURF> extractor = SURF::create();

	Mat descriptors_object, descriptors_scene;

	extractor->compute(img_object, keypoints_object, descriptors_object);
	extractor->compute(img_scene, keypoints_scene, descriptors_scene);

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match(descriptors_object, descriptors_scene, matches);

	
	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	std::vector< DMatch > good_matches;

	const float min_dist_allowed = min_dist_tolerance * min_dist;
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		//float dist_y = abs(keypoints_object[matches[i].queryIdx].pt.y -
		//	keypoints_scene[matches[i].trainIdx].pt.y);

		if (matches[i].distance < min_dist_allowed) // && dist_y < max_distance_y)
		{
			good_matches.push_back(matches[i]);
		}
	}

	Mat img_matches;
	drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
		good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
		std::vector<char>(), DrawMatchesFlags::DEFAULT);

	//-- Localize the object
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	std::vector<KeyPoint> keypoints_1, keypoints_2;

	for (int i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);

		keypoints_1.push_back(keypoints_object[good_matches[i].queryIdx]);
		keypoints_2.push_back(keypoints_scene[good_matches[i].trainIdx]);
	}

	Mat H = findHomography(obj, scene, RANSAC);

	//-- Get the corners from the image_1 ( the object to be "detected" )
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(img_object.cols, 0);
	obj_corners[2] = cvPoint(img_object.cols, img_object.rows); obj_corners[3] = cvPoint(0, img_object.rows);
	std::vector<Point2f> scene_corners(4);

	perspectiveTransform(obj_corners, scene_corners, H);

	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	//line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 255), 4);
	//line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 255), 4);
	//line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 255), 4);
	//line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 255), 4);

	//-- Draw keypoints
	//Mat img_keypoints_1; Mat img_keypoints_2;
	//drawKeypoints(img_object, keypoints_1, img_keypoints_1, Scalar(0, 255, 255), DrawMatchesFlags::DEFAULT);
	//drawKeypoints(img_scene, keypoints_2, img_keypoints_2, Scalar(0, 255, 255), DrawMatchesFlags::DEFAULT);

	//show_window("img_keypoints_1", img_keypoints_1);
	//show_window("img_keypoints_2", img_keypoints_2);
	show_window("img_matches", img_matches);
	
	//imwrite("img_1.png", img_keypoints_1);
	//imwrite("img_2.png", img_keypoints_2);
	imwrite("img_matches.png", img_matches);

	
	std::cout << "All Matches  : " << matches.size() << std::endl;
	std::cout << "Good Matches : " << good_matches.size() << std::endl;
	std::cout << "keypoints_1 : " << keypoints_1.size() << std::endl;
	std::cout << "keypoints_2 : " << keypoints_2.size() << std::endl;

	waitKey(0);

}

/** @function readme */
void readme()
{
	std::cout << " Usage: ./SURF_detector <img1> <img2>" << std::endl;
}
