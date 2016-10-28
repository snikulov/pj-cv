// PizzaJones.cpp : Defines the entry point for the console application.
//

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


Mat RemoveBackground(Mat src)
{
	if (src.channels() > 1)
		cv::cvtColor(src, src, CV_BGR2BGRA);

	// find all white pixel and set alpha value to zero:
	for (int y = 0; y < src.rows; ++y)
		for (int x = 0; x < src.cols; ++x)
		{
			cv::Vec4b & pixel = src.at<cv::Vec4b>(y, x);
			// if pixel is white
			if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255)
			{
				// set alpha to zero:
				pixel[3] = 0;
			}
		}
	return src;
}

Mat GetGradient(Mat src)
{
	if (src.channels() > 1)
		cvtColor(src, src, cv::COLOR_RGB2GRAY);
	Mat grad;

	return src;

	/*

	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_32FC1; ;

	// Calculate the x and y gradients using Sobel operator

	Sobel(src, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	Sobel(src, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	// Combine the two gradients
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	*/

	return grad;

}

int main(int argc, char** argv)
{
	// known issues
	// 1. some objects may be detected as pizza - need to combine with HOG?
	// 2. some pizzas are wrong size, i.e. need to define what size should I use

	// load test images, TODO: replace to load it from file list
	int num_images = 11;
	Mat test_image[11];
	test_image[0] = imread("test-1.jpg", IMREAD_COLOR); // Read the file
	test_image[1] = imread("test-2.jpg", IMREAD_COLOR); // Read the file
	test_image[2] = imread("test-3.jpg", IMREAD_COLOR); // Read the file
	test_image[3] = imread("test-4.jpg", IMREAD_COLOR); // Read the file
	test_image[4] = imread("test-5.jpg", IMREAD_COLOR); // Read the file
	test_image[5] = imread("test-6.jpg", IMREAD_COLOR); // Read the file
	test_image[6] = imread("test-7.jpg", IMREAD_COLOR); // Read the file
	test_image[7] = imread("test-8.jpg", IMREAD_COLOR); // Read the file
	test_image[8] = imread("test-9.jpg", IMREAD_COLOR); // Read the file
	test_image[9] = imread("test-negative-1.jpg", IMREAD_COLOR); // Read the file
	test_image[10] = imread("test-negative-2.jpg", IMREAD_COLOR); // Read the file
	// load template and mask, for future there should be several templates and masks
	// also we should have differnt mask for different sizes of pizza
	Mat template_image = imread("pizza-large-template-1.jpg", IMREAD_COLOR); // Read the file
	Mat mask_image = imread("pizza-large-mask-1.jpg", IMREAD_COLOR); // Read the file

	//scale down image to speedup matching
	double image_scale = 0.25;
	cv::resize(template_image, template_image, cv::Size(0,0), image_scale, image_scale, INTER_AREA);
	cv::resize(mask_image, mask_image, cv::Size(0, 0), image_scale, image_scale, INTER_AREA);
	//template_image = RemoveBackground(template_image);
	//mask_image = RemoveBackground(mask_image);

	for (int i = 0; i < num_images; i++)
	{
		// scale down image
		//cv::cvtColor(test_image[i], test_image[i], CV_BGR2BGRA);
		cv::resize(test_image[i], test_image[i], cv::Size(0, 0), image_scale, image_scale, INTER_AREA);

		// increase size of image to capture part of the pizza.
		Mat img(test_image[i].rows*1.5, test_image[i].cols*1.5, test_image[i].type(), Scalar(125, 125, 125));

		Mat imgRoi(img, Rect((img.cols - test_image[i].cols) / 2, (img.rows - test_image[i].rows) / 2, test_image[i].cols, test_image[i].rows));
		// copy to center
		test_image[i].copyTo(imgRoi);

		// resulted image for matching
		Mat result; result.create(img.rows - template_image.rows + 1, img.cols - template_image.cols + 1, CV_32FC1);

		// CV_TM_SQDIFF ok
		// CV_TM_CCOEFF_NORMED works ok
		// CV_TM_CCORR_NORMED ok with white bg
		int match_method = CV_TM_CCORR_NORMED;
		cv::matchTemplate(img, template_image, result, match_method);// , mask_image);




		double minVal; double maxVal;
		// Find Min and Max to use as detector.
		cv::minMaxIdx(result, &minVal, &maxVal);
		// TODO: add here calculate of sigma to use it as PIZZE detaction method.

		cv::normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

		Point matchLoc; Point minLoc; Point maxLoc; double matchVal;

		minMaxLoc(result, (double *)0, (double *)0, &minLoc, &maxLoc, Mat());

		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
			matchVal = minVal;
		}
		else
		{
			matchLoc = maxLoc;
			matchVal = maxVal;
		}

#if 0
		// BEGIN finetune adjustments
		// Set the stopping criteria for the algorithm.
		int number_of_iterations = 500;
		double termination_eps = 1e-10;

		// Define motion model
		const int warp_mode = MOTION_AFFINE;

		// Set space for warp matrix.
		Mat warp_matrix;

		// Set the warp matrix to identity.
		if (warp_mode == MOTION_HOMOGRAPHY)
			warp_matrix = Mat::eye(3, 3, CV_32F);
		else
			warp_matrix = Mat::eye(2, 3, CV_32F);

		TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, number_of_iterations, termination_eps);

		Mat imgroi2(img, Rect(Point(matchLoc.x, matchLoc.y), Point(matchLoc.x + template_image.cols, matchLoc.y + template_image.rows)));

		// TODO: add exception handler as this fun
		double cc = findTransformECC(GetGradient(template_image), GetGradient(imgroi2), warp_matrix, warp_mode, criteria);

		cout << "warp_matrix : " << warp_matrix << endl;
		cout << "CC " << cc << endl;

		Mat template_image_wrapped, mask_image_wrapped;
		if (warp_mode == MOTION_HOMOGRAPHY)
		{
			// Use Perspective warp when the transformation is a Homography
			warpPerspective(template_image, template_image_wrapped, warp_matrix, imgroi2.size(), INTER_LINEAR + WARP_INVERSE_MAP);
			warpPerspective(mask_image, mask_image_wrapped, warp_matrix, imgroi2.size(), INTER_LINEAR + WARP_INVERSE_MAP);
			template_image_wrapped.copyTo(imgroi2, mask_image_wrapped);
		}
		else
		{
			// Use Affine warp when the transformation is not a Homography
			//warpAffine(template_image, imgroi2, warp_matrix, imgroi2.size(), INTER_LINEAR + WARP_INVERSE_MAP);
			warpAffine(template_image, template_image_wrapped, warp_matrix, imgroi2.size(), INTER_LINEAR + WARP_INVERSE_MAP);
			warpAffine(mask_image, mask_image_wrapped, warp_matrix, imgroi2.size(), INTER_LINEAR + WARP_INVERSE_MAP);
			template_image_wrapped.copyTo(imgroi2, mask_image_wrapped);
		}
			
		// END finetune
#endif
		Mat img_display; img.copyTo(img_display);
		rectangle(img_display, matchLoc, Point(matchLoc.x + template_image.cols, matchLoc.y + template_image.rows), Scalar::all(0), 2, 8, 0);
		putText(img_display, cv::String(std::to_string(matchVal)), matchLoc, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0));
		rectangle(result, matchLoc, Point(matchLoc.x + template_image.cols, matchLoc.y + template_image.rows), Scalar::all(0), 2, 8, 0);

		imshow("Source Image", img_display);
		imshow("result Window", result);
		waitKey(0); // Wait for a keystroke in the window

	}

	return 0;
}
