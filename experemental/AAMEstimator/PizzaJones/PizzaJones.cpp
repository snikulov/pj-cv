// PizzaJones.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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

Mat RemoveBackground2(Mat src)
{
	return src;
	// find all white pixel and set alpha value to zero:
	for (int y = 0; y < src.rows; ++y)
		for (int x = 0; x < src.cols; ++x)
		{
			cv::Vec3b & pixel = src.at<cv::Vec3b>(y, x);
			// if pixel is white
			if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255)
			{
				pixel[0] = pixel[1] = pixel[2] = 0;
			}
		}
	return src;
}


Mat GetGradient(Mat src)
{
	if (src.channels() > 1)
		cvtColor(src, src, cv::COLOR_RGB2GRAY);

	Mat grad = src;
#if 0
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
#endif
	// th3 = cv2.adaptiveThreshold(img,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
   12             cv2.THRESH_BINARY,11,2)
	return grad;

}

class Pizza :public cv::MinProblemSolver::Function {
private:
	const int dims = 8;
public:
	Mat template_image, img, mask_image;
	Point loc;

	//Mat x; left top x,y; right top x,y; right bottom x,y
	Mat x0 = (cv::Mat_<double>(1, dims) << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	//Mat step = (cv::Mat_<double>(dims, 1) << -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	Mat step = (cv::Mat_<double>(dims, 1) << 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);


	void init()
	{
		x0.at<double>(0, 0) = loc.x;
		x0.at<double>(0, 1) = loc.y;
		x0.at<double>(0, 2) = loc.x + template_image.cols;
		x0.at<double>(0, 3) = loc.y;
		x0.at<double>(0, 4) = loc.x + template_image.cols;
		x0.at<double>(0, 5) = loc.y + template_image.rows;
		x0.at<double>(0, 6) = loc.x;
		x0.at<double>(0, 7) = loc.y + template_image.rows;
	}

	int getDims() const { return dims; }
	double calc(const double* x)const {
		// build points
		cv::Point2f pts_src[4], pts_dst[4];
		pts_src[0].x = 0; pts_src[0].y = 0; //top left
		pts_src[1].x = template_image.cols; pts_src[1].y = 0; //top right
		pts_src[2].x = template_image.cols; pts_src[2].y = template_image.rows; //bottom right
		pts_src[3].x = 0; pts_src[3].y = template_image.rows; //bottom left

																				/*
																				pts_dst[0].x = loc.x; pts_dst[0].y = loc.y; //top left
																				pts_dst[1].x = loc.x + template_image.cols; pts_dst[1].y =loc.y; //top right
																				pts_dst[2].x = loc.x + template_image.cols; pts_dst[2].y = loc.y + template_image.rows; //bottom right
																				*/
		pts_dst[0].x = x[0]; pts_dst[0].y = x[1]; //top left
		pts_dst[1].x = x[2]; pts_dst[1].y = x[3]; //top right
		pts_dst[2].x = x[4]; pts_dst[2].y = x[5]; //bottom right
		pts_dst[3].x = x[6]; pts_dst[3].y = x[7]; //bottom left


		Mat warp_matrix = cv::getPerspectiveTransform(pts_src, pts_dst);
		cout << "warp_matrix : " << warp_matrix << endl;

		cout << "X : " << x[0] << " " << x[1] << " " << x[2] << " " << x[3] << " " << x[4] << " " << x[5] << " " << x[6] << " " << x[7] << endl;

		Mat template_image_wrapped, mask_image_wrapped;
		warpPerspective(template_image, template_image_wrapped, warp_matrix, img.size());// , INTER_LINEAR + WARP_INVERSE_MAP);
		warpPerspective(mask_image, mask_image_wrapped, warp_matrix, img.size());// , INTER_LINEAR + WARP_INVERSE_MAP);
																			//template_image_wrapped.copyTo(img, mask_image_wrapped);


		Mat img2compare; img.copyTo(img2compare, mask_image_wrapped);
		//img2compare = RemoveBackground(img2compare);
		//Mat template_image_wrapped2 = RemoveBackground2(template_image_wrapped);

		//template_image_wrapped2 = GetGradient(template_image_wrapped2);
		//Mat img2compare2 = GetGradient(img2compare);

		imshow("2", img2compare);
		imshow("1", template_image_wrapped);

		// CV_TM_SQDIFF ok
		// CV_TM_CCOEFF_NORMED works ok
		// CV_TM_CCORR_NORMED ok with white bg
		int match_method = CV_TM_CCORR_NORMED;
		Mat result; result.create(1, 1, CV_32FC1);

		//GaussianBlur(img2compare, img2compare, Size(5, 5), 0.0);
		//GaussianBlur(template_image_wrapped, template_image_wrapped, Size(5, 5), 0.0);


		cv::matchTemplate(GetGradient(img2compare), GetGradient(template_image_wrapped), result, match_method, GetGradient(mask_image_wrapped));

		cout << "result : " << result << endl;

		Mat img_t = img.clone();
		template_image_wrapped.copyTo(img_t, mask_image_wrapped);

		double minVal; double maxVal;
		cv::minMaxIdx(result, &minVal, &maxVal);

		imshow("Source Image", img_t);
		//imshow("result Window", result);
		waitKey(10); // Wait for a keystroke in the window

		return 1-minVal;
	}
};

int main(int argc, char** argv)
{
	// known issues
	// 1. some objects may be detected as pizza - need to combine with HOG?
	// 2. some pizzas are wrong size, i.e. need to define what size should I use

	// load test images, TODO: replace to load it from file list
	int num_images = 11;
	Mat test_image[11];
	test_image[0] = imread("test-8.jpg", IMREAD_COLOR); // Read the file
	test_image[1] = imread("test-2.jpg", IMREAD_COLOR); // Read the file
	test_image[2] = imread("test-3.jpg", IMREAD_COLOR); // Read the file
	test_image[3] = imread("test-4.jpg", IMREAD_COLOR); // Read the file
	test_image[4] = imread("test-5.jpg", IMREAD_COLOR); // Read the file
	test_image[5] = imread("test-6.jpg", IMREAD_COLOR); // Read the file
	test_image[6] = imread("test-7.jpg", IMREAD_COLOR); // Read the file
	test_image[7] = imread("test-1.jpg", IMREAD_COLOR); // Read the file
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
		const int warp_mode = MOTION_HOMOGRAPHY; // MOTION_AFFINE or MOTION_HOMOGRAPHY

		// Set space for warp matrix.
		Mat warp_matrix;

		// Set the warp matrix to identity.
		if (warp_mode == MOTION_HOMOGRAPHY)
			warp_matrix = Mat::eye(3, 3, CV_32F);
		else
			warp_matrix = Mat::eye(2, 3, CV_32F);

		TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, number_of_iterations, termination_eps);

		Mat imgroi2(img, Rect(Point(matchLoc.x-10, matchLoc.y-10), Point(matchLoc.x + template_image.cols + 10, matchLoc.y + template_image.rows+10)));

		//GaussianBlur(imgroi2, imgroi2, Size(5, 5),0.0);
		//GaussianBlur(template_image, template_image, Size(5, 5),0.0);

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


#if 1
		/*
		int number_of_iterations = 500;
		double termination_eps = 1e-10;
		TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, number_of_iterations, termination_eps);
		*/
		cv::Ptr<cv::DownhillSolver> solver = cv::DownhillSolver::create();

		//cv::Ptr<cv::MinProblemSolver::Function> ptr_F = cv::makePtr<Pizza>();
		Pizza pizza_C = Pizza();
		pizza_C.img = img;
		pizza_C.template_image = template_image;
		pizza_C.mask_image = mask_image;
		pizza_C.loc = matchLoc;
		pizza_C.init();
		cv::Ptr<cv::MinProblemSolver::Function> ptr_F = cv::makePtr<Pizza>(pizza_C);





		solver->setFunction(ptr_F);
		solver->setInitStep(pizza_C.step);
		double res = solver->minimize(pizza_C.x0);
		double tol = solver->getTermCriteria().epsilon;

#endif

		Mat img_display; img.copyTo(img_display);
		rectangle(img_display, matchLoc, Point(matchLoc.x + template_image.cols, matchLoc.y + template_image.rows), Scalar::all(0), 2, 8, 0);
		putText(img_display, cv::String(std::to_string(matchVal)), matchLoc, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0));
		//rectangle(result, matchLoc, Point(matchLoc.x + template_image.cols, matchLoc.y + template_image.rows), Scalar::all(0), 2, 8, 0);

		imshow("Source Image", img_display);
		//imshow("result Window", result);
		waitKey(0); // Wait for a keystroke in the window

	}

	return 0;
}
