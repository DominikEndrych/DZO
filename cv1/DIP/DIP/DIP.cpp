// DIP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

cv::Mat GrayCorrection(cv::Mat original, double newMin, double newMax)
{
	cv::Mat result = original.clone();	// Result image

	double oldMin, oldMax;
	cv::minMaxLoc(original, &oldMin, &oldMax);	// Find minimum and maximum on original grayscale

	double oldRange = oldMax - oldMin;		// Old range of grayscale
	double newRange = newMax - newMin;		// New range of grayscale

	for(int r = 0; r < original.rows; r++)
	{
		for (int c = 0; c < original.cols; c++)
		{
			double currentValue = result.at<uchar>(r, c);	// Current value in pixel
			result.at<uchar>(r, c) = (((currentValue - oldMin) * newRange) / oldRange) + newMin;	// Conversion to a new rage
		}
	}

	return result;
}

cv::Mat GammaCorrection(cv::Mat original, double gamma)
{
	cv::Mat result = original.clone();

	double gammaExp = 1 / gamma;

	for (int r = 0; r < original.rows; r++)
	{
		for (int c = 0; c < original.cols; c++)
		{
			double currentValue = result.at<uchar>(r, c);	// Current value in pixel
			result.at<uchar>(r, c) = pow(currentValue / 255, gammaExp) * 255;	// Gamma correction
		}
	}

	return result;

	// NOTE: Gamma of value 1 should not change anything
}

cv::Mat Concolution(cv::Mat original, cv::Mat mask, float coeficient) 
{
	//cv::Mat result(original.rows, original.cols, CV_8UC1, 0);
	cv::Mat result = original.clone();

	int border = mask.cols / 2;		// Border for given mask

	//printf("%d %d \n", result.rows, result.cols);
	
	for(int c=0; c<original.cols; c++)
	{
		for (int r = 0; r < original.rows; r++)
		{
			cv::Point leftCorner = cv::Point(c-border, r-border);		// Top left corner
			cv::Point rightCorner = cv::Point(c+border+1, r+border+1);	// Bottom right corner
			cv::Rect area = cv::Rect(leftCorner, rightCorner);

			//printf("%d - %d\n", leftCorner.x, leftCorner.y);

			if (leftCorner.x > 0 && leftCorner.y > 0 && rightCorner.x < result.cols + 1 && rightCorner.y < result.rows + 1)
			{
				
				//printf("%d - %d\n", leftCorner.x, leftCorner.y);
				cv::Mat croppedArea = original(area);	// Area cropped by mask
				//printf("Rows = %d\nCols = %d\n", croppedArea.rows, croppedArea.cols);

				cv::Mat newArea;					// Area with mask applied
				newArea = croppedArea.mul(mask);
				newArea = newArea/coeficient;

				
				cv::Scalar newPixel = cv::sum(newArea);	// Scalar for final pixel
				float amount = cv::sum(newPixel)[0];	// Value in pixel
				result.at<uchar>(r, c) = amount;
			}
		}
	}

	return result;
}


int main()
{
	cv::Mat src_8uc3_img = cv::imread("images/lena.png", CV_LOAD_IMAGE_COLOR); // load color image from file system to Mat variable, this will be loaded using 8 bits (uchar)

	#pragma region Cviceni 1

	// Cv1 - grayscale correction and gamma
	cv::Mat moonOrig_8uc1_img = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_GRAYSCALE);	// Load moon image
	cv::Mat moon_8uc1_img;

	if (moonOrig_8uc1_img.empty()) {
		printf("Unable to read input file (%s, %d).", __FILE__, __LINE__);
	}

	moon_8uc1_img = GrayCorrection(moonOrig_8uc1_img, 0, 255);					// Grayscale correction to 0-255 range

	cv::imshow("Moon original", moonOrig_8uc1_img);								// Original image
	cv::imshow("Moon grayscale correction", moon_8uc1_img);
	cv::imshow("Moon - 2.4 gamma", GammaCorrection(moon_8uc1_img, 2.4));		// Moon image with 2.4 gamma
	cv::imshow("Moon - 0.2 gamma", GammaCorrection(moon_8uc1_img, 0.2));		// Moon image witch 0.2 gamma
	

	#pragma endregion grayscale and gamma correction

	#pragma region Cviceni 2
	uchar boxBlurData[] = { 1,1,1,1,1,1,1,1,1 };	// Values for box blur mask
	cv::Mat boxBlur(3, 3, CV_8UC1, boxBlurData);	// Create mask for box blur

	cv::imshow("Maybe box blur", Concolution(moon_8uc1_img, boxBlur, 9));	// Convolution with box blur
	#pragma endregion convolution

	
	

	/*
	if (src_8uc3_img.empty()) {
		printf("Unable to read input file (%s, %d).", __FILE__, __LINE__);
	}

	//cv::imshow( "LENA", src_8uc3_img );

	cv::Mat gray_8uc1_img; // declare variable to hold grayscale version of img variable, gray levels wil be represented using 8 bits (uchar)
	cv::Mat gray_32fc1_img; // declare variable to hold grayscale version of img variable, gray levels wil be represented using 32 bits (float)

	cv::cvtColor(src_8uc3_img, gray_8uc1_img, CV_BGR2GRAY); // convert input color image to grayscale one, CV_BGR2GRAY specifies direction of conversion
	gray_8uc1_img.convertTo(gray_32fc1_img, CV_32FC1, 1.0 / 255.0); // convert grayscale image from 8 bits to 32 bits, resulting values will be in the interval 0.0 - 1.0

	int x = 10, y = 15; // pixel coordinates

	uchar p1 = gray_8uc1_img.at<uchar>(y, x); // read grayscale value of a pixel, image represented using 8 bits
	float p2 = gray_32fc1_img.at<float>(y, x); // read grayscale value of a pixel, image represented using 32 bits
	cv::Vec3b p3 = src_8uc3_img.at<cv::Vec3b>(y, x); // read color value of a pixel, image represented using 8 bits per color channel

	// print values of pixels
	printf("p1 = %d\n", p1);
	printf("p2 = %f\n", p2);
	printf("p3[ 0 ] = %d, p3[ 1 ] = %d, p3[ 2 ] = %d\n", p3[0], p3[1], p3[2]);

	gray_8uc1_img.at<uchar>(y, x) = 0; // set pixel value to 0 (black)

	// draw a rectangle
	cv::rectangle(gray_8uc1_img, cv::Point(65, 84), cv::Point(75, 94),
		cv::Scalar(50), CV_FILLED);

	// declare variable to hold gradient image with dimensions: width= 256 pixels, height= 50 pixels.
	// Gray levels wil be represented using 8 bits (uchar)
	cv::Mat gradient_8uc1_img(50, 256, CV_8UC1);

	// For every pixel in image, assign a brightness value according to the x coordinate.
	// This wil create a horizontal gradient.
	for (int y = 0; y < gradient_8uc1_img.rows; y++) {
		for (int x = 0; x < gradient_8uc1_img.cols; x++) {
			gradient_8uc1_img.at<uchar>(y, x) = x;
		}
	}

	// diplay images
	cv::imshow("Gradient", gradient_8uc1_img);
	cv::imshow("Lena gray", gray_8uc1_img);
	cv::imshow("Lena gray 32f", gray_32fc1_img);
	*/

	cv::waitKey(0); // wait until keypressed

	return 0;
}
