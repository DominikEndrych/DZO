// DIP.cpp : Defines the entry point for the console application.
//
#define _USE_MATH_DEFINES
#include <math.h>
#include "stdafx.h"
#include "interpolation.h"

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
	cv::Mat result = original.clone();

	int border = mask.cols / 2;		// Border for given mask
	
	for(int c=0; c<original.cols; c++)
	{
		for (int r = 0; r < original.rows; r++)
		{
			cv::Point leftCorner = cv::Point(c-border, r-border);		// Top left corner
			cv::Point rightCorner = cv::Point(c+border+1, r+border+1);	// Bottom right corner
			cv::Rect area = cv::Rect(leftCorner, rightCorner);

			if (leftCorner.x > 0 && leftCorner.y > 0 && rightCorner.x < result.cols + 1 && rightCorner.y < result.rows + 1)
			{
				cv::Mat croppedArea = original(area);	// Area cropped by mask

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

cv::Mat AnisotropicFiltration(cv::Mat original, float s, float l)
{
	double sigma = 0.015;
	double delta = 0.1;

	cv::Mat orig;
	original.copyTo(orig);

	cv::Mat result;
	orig.copyTo(result);

	for (int k = 0; k < 100; k++) {

		for (int r = 1; r < original.rows - 1; r++) {
			for (int c = 1; c < original.cols - 1; c++) {

				// Delta calculations
				double del_C_N = orig.at<double>(r, c - 1) - orig.at<double>(r, c);
				double del_C_S = orig.at<double>(r, c + 1) - orig.at<double>(r, c);
				double del_C_E = orig.at<double>(r + 1, c) - orig.at<double>(r, c);
				double del_C_W = orig.at<double>(r - 1, c) - orig.at<double>(r, c);

				double C_N = abs(exp(-1 * (del_C_N * del_C_N / (sigma * sigma))));
				double C_S = abs(exp(-1 * (del_C_S * del_C_S / (sigma * sigma))));
				double C_E = abs(exp(-1 * (del_C_E * del_C_E / (sigma * sigma))));
				double C_W = abs(exp(-1 * (del_C_W * del_C_W / (sigma * sigma))));

				// Result color
				result.at<double>(r, c) = orig.at<double>(r, c) * (1 - delta * (C_N + C_S + C_E + C_W)) +
					delta * (C_N * orig.at<double>(r, c - 1) + C_S * orig.at<double>(r, c + 1)
						+ C_E * orig.at<double>(r + 1, c) + C_W * orig.at<double>(r - 1, c));
			}
		}
		result.copyTo(orig);
	}
	return result;
}

void ShowFourier(const cv::Mat& complexMatrix)
{
	cv::Mat powerImg(complexMatrix.rows, complexMatrix.cols, CV_64FC1);
	cv::Mat phaseImg;
	powerImg.copyTo(phaseImg);

	int width = complexMatrix.cols;
	int height = complexMatrix.rows;

	for (int k = 0; k < height; k++)
	{
		for (int l = 0; l < width; l++)
		{
			cv::Vec2d cArray = complexMatrix.at<cv::Vec2d>(k, l);
			double realPart = cArray[0];
			double complexPart = cArray[1];

			double spectrum = sqrt(realPart * realPart + complexPart * complexPart);
			double power = spectrum * spectrum;
			double phase = atan(complexPart / realPart);
			double lg = log2(power);
			phaseImg.at<double>(k, l) = phase;
			powerImg.at<double>(k, l) = lg;
		}
	}

	double min, max;
	cv::minMaxLoc(powerImg, &min, &max);
	double scale = max - min;

	for (int r = 0; r < height / 2; r++)
	{
		for (int c = 0; c < width; c++)
		{
			int x = (c < width / 2) ? (c + width / 2) : (c - width / 2);
			int y = r + height / 2;
			std::swap(powerImg.at<double>(r, c), powerImg.at<double>(y, x));
			powerImg.at<double>(r, c) = (powerImg.at<double>(r, c) - min) / scale;
			powerImg.at<double>(y, x) = (powerImg.at<double>(y, x) - min) / scale;
		}
	}

	//cv::imshow("Phase", phaseImg);
	cv::imshow("Power", powerImg);
}

void SwapQuadrants(cv::Mat& img)
{
	int width = img.cols;
	int height = img.rows;

	for (int k = 0; k < height / 2; k++)
	{
		for (int l = 0; l < width; l++)
		{
			int x;
			if (width / 2 > l) { x = l + width / 2; }
			else { x = l - width / 2; }

			int y = k + height / 2;

			std::swap(img.at<cv::Vec2d>(k, l), img.at<cv::Vec2d>(y, x));
		}
	}
}

cv::Mat DiscreteFourierTransform(const cv::Mat& original)
{
	cv::Mat result(original.rows, original.cols, CV_64FC2);

	int width = original.cols;
	int height = original.rows;

	double scaleMN = 1.0 / sqrt(original.rows * original.cols);

	for (int r = 0; r < height; r++)
	{
		for (int c = 0; c < width; c++)
		{
			double realPart = 0.0, complexPart = 0.0;

			for (int m = 0; m < height; m++)
			{
				for (int n = 0; n < width; n++)
				{
					double pixel = original.at<double>(m, n);
					double eVal = 2 * 3.14159265359 * ((((double)m * (double)r) / (double)height) + (((double)n * (double)c) / (double)width));
					realPart += std::cos(-eVal) * pixel;		// Real part of complex number
					complexPart += std::sin(-eVal) * pixel;		// Complex part of complex number
				}
			}

			result.at<cv::Vec2d>(r, c) = cv::Vec2d(realPart, complexPart);
		}
	}

	return result;
}
cv::Mat InvertedDiscreteFourierTransform(const cv::Mat& complexMat)
{
	cv::Mat result(complexMat.rows, complexMat.cols, CV_64FC1);

	int width = complexMat.cols;
	int height = complexMat.rows;

	double scale = 1.0 / sqrt(width * height);

	for (int r = 0; r < height; r++)
	{
		for (int c = 0; c < width; c++)
		{
			double real = 0.0;

			for (int m = 0; m < height; m++)
			{
				for (int n = 0; n < width; n++)
				{
					cv::Vec2d coef = complexMat.at<cv::Vec2d>(m, n);
					double eVal = 2 * 3.14159265359 * ((((double)m * (double)r) / (double)height) + (((double)n * (double)c) / (double)width));
					double bReal = scale * std::cos(eVal);	
					double bComplex = scale * std::sin(eVal);
					real += (coef[0] * bReal) - (coef[1] * bComplex);		// real part
				}
			}

			result.at<double>(r, c) = real;
		}
	}

	return result;
}

void ApplyMask(cv::Mat& complexMat, cv::Mat& mask)
{
	SwapQuadrants(complexMat);
	for (int r = 0; r < mask.rows; r++)
	{
		for (int c = 0; c < mask.cols; c++)
		{
			if(mask.at<uchar>(r,c) < 1 )
			{
				complexMat.at<cv::Vec2d>(r, c) = cv::Vec2d(0.0, 0.0);
			}
		}
	}
	SwapQuadrants(complexMat);
}


float TaylorSeries(float k1, float k2, float r2)
{
	// r2 here is r2 = r * r
	return 1.0f + k1 * r2 + k2 * pow(r2, 3);
}


cv::Mat Undistort(cv::Mat original, float k1, float k2)
{
	cv::Mat result;
	original.copyTo(result);

	float center_x = result.rows / 2;
	float center_y = result.cols / 2;
	float R = sqrt((center_x * center_x) + (center_y * center_y));	// R for dimensionless coordinates formula

	for (int x_n = 0; x_n < original.rows; x_n++)
	{
		for (int y_n = 0; y_n < original.cols; y_n++)
		{

			float _x = (x_n - center_x) / R;
			float _y = (y_n - center_y) / R;

			float r2 = pow(_x, 2) + pow(_y, 2);				// r2 for Taylor Series
			float theta = 1.0f / TaylorSeries(k1, k2, r2);	// Theta function in formula

			float x_d = (x_n - center_x) * theta + center_x;
			float y_d = (y_n - center_y) * theta + center_y;

			if (x_d >= 0 && x_d < original.rows && y_d >= 0 && y_d < original.cols)
			{
				result.at<cv::Vec3b>(x_n, y_n) = bilinearInterpolation<cv::Vec3b>(original, x_d, y_d);	// Bilinear interpolation is used
			}
			else result.at<cv::Vec3b>(x_n, y_n) = cv::Vec3b(0, 0, 0);	// Out of the image
		}
	}

	return result;
}

void GetHistogram(cv::Mat& img, int* result)
{
	// NOTE: result must be array of zeros

	for (int i = 0; i < 256; i++)
	{
		result[i] = 0;
	}

	for (int r = 0; r < img.rows; r++)
	{
		for (int c = 0; c < img.cols; c++)
		{
			result[img.at<uchar>(r, c)]++;	// Increment value in histogram
		}
	}
}


void ShowHistogram(const char* windowName, int* histogram)
{
	int min, max;
	max = min = histogram[0];

	for (int i = 1; i < 256; i++)
	{
		if (histogram[i] < min) { min = histogram[i]; }
		if (histogram[i] > max) { max = histogram[i]; }
	}

	cv::Mat histogramImg(256, 256, CV_32FC1);	// Mat for result image

	for (int i = 0; i < histogramImg.cols; i++)
	{
		float height = (float)(histogram[i] - min);
		height /= (max - min);

		if (height < 0.0f) height = 0.0f;

		for (int j = 0; j < histogramImg.rows; j++)
		{
			if (j < height * histogramImg.rows)
			{
				histogramImg.at<float>(histogramImg.rows - j - 1, i) = 1.0f;	// Draw white
			}
			else histogramImg.at<float>(histogramImg.rows - j - 1, i) = 0.0f;	// Draw black
		}
	}

	cv::imshow(windowName, histogramImg);
}

int HistogramCdf(int* histogram, int i)
{
	int result = 0;

	for (int j = 0; j <= i; j++)
	{
		result += histogram[j];
	}

	return result;
}

int GetNonZeroMin(int* inputArray)
{
	int i = 0;

	int result = inputArray[i];

	// If first value is zero, find the closest non-zelo value
	while (result == 0 && i < 256)
	{
		i++;
		result = inputArray[i];
	}

	// Continue from first undiscovered position
	for (int j = i; j < 256; j++)
	{
		if (result > inputArray[j]) { result = inputArray[j]; }
	}

	return result;
}



void EqualizeHistogram(cv::Mat& img, int* histogram)
{
	int cdfArray[256];		// cdf function values
	float results[256];		// Look-up table

	for (int i = 0; i < 256; i++)
	{
		cdfArray[i] = HistogramCdf(histogram, i);
	}

	int cdf_min = GetNonZeroMin(cdfArray);	// Find non zero minimum

	for (int i = 0; i < 256; i++)
	{
		results[i] = ((float)(cdfArray[i] - cdf_min) / ((img.rows * img.cols) - cdf_min)) * 255;	// Construct look-up table
	}

	for (int r = 0; r < img.rows; r++)
	{
		for (int c = 0; c < img.cols; c++)
		{
			img.at<uchar>(r, c) = results[img.at<uchar>(r, c)];		// Find result value in pixel using look-up table
		}
	}
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

	//cv::imshow("Moon original", moonOrig_8uc1_img);								// Original image
	//cv::imshow("Moon grayscale correction", moon_8uc1_img);
	//cv::imshow("Moon - 2.4 gamma", GammaCorrection(moon_8uc1_img, 2.4));		// Moon image with 2.4 gamma
	//cv::imshow("Moon - 0.2 gamma", GammaCorrection(moon_8uc1_img, 0.2));		// Moon image witch 0.2 gamma
	

	#pragma endregion grayscale and gamma correction

	#pragma region Cviceni 2
	uchar boxBlurData[] = { 1,1,1,1,1,1,1,1,1 };	// Values for box blur mask
	cv::Mat boxBlur(3, 3, CV_8UC1, boxBlurData);	// Create mask for box blur

	//cv::imshow("Maybe box blur", Concolution(moon_8uc1_img, boxBlur, 9));	// Convolution with box blur

	#pragma endregion convolution

	#pragma region Cviceni 3
	cv::Mat valve_8uc1_img = cv::imread("images/valve.png", CV_LOAD_IMAGE_GRAYSCALE);	// Load image

	if (valve_8uc1_img.empty()) {
		printf("Unable to read input file (%s, %d).", __FILE__, __LINE__);
		return 0;
	}

	cv::Mat valve_64fc1_img;
	valve_8uc1_img.convertTo(valve_64fc1_img, CV_64FC1, 1.0 / 255.0);	// Convert image to Double grayscale

	/*
	cv::imshow("Before", valve_64fc1_img);
	cv::imshow("Anisotropic", AnisotropicFiltration(valve_64fc1_img, 0.1, 0.1));
	*/
	#pragma endregion Anisotropic diffusion

	#pragma region Cviceni 4 + 5

	cv::Mat lena64_gray = cv::imread("images/lena64_noise.png", CV_LOAD_IMAGE_GRAYSCALE);	// Load image as grayscale
	//cv::imshow("Orig", lena64_gray);
	double normMN = 1.0 / sqrt(lena64_gray.rows * lena64_gray.cols);	
	lena64_gray.convertTo(lena64_gray, CV_64FC1, (1.0 / 255.0) * normMN);	// Scale image

	cv::Mat complexMatrix = DiscreteFourierTransform(lena64_gray);	// Transform image to coeficients with DFT

	// Create mask
	cv::Mat mask(lena64_gray.rows, lena64_gray.cols, CV_8UC1, cv::Scalar(0));	// Black image for the mask
	float centerX = lena64_gray.cols / 2.0f;	// X coordinate of circle
	float centerY = lena64_gray.rows / 2.0f;	// Y coordinate of circle
	int radius = 17;							// Radius of the circle
	cv::circle(mask, cv::Point(centerX, centerY), radius, cv::Scalar(255), -1);	// Draw circle
	//cv::imshow("Mask", mask);

	//ShowFourier(complexMatrix);

	ApplyMask(complexMatrix, mask);

	cv::Mat restored = InvertedDiscreteFourierTransform(complexMatrix);

	//cv::imshow("Restored", restored);
	//cv::resizeWindow("Restored", 200, restored.rows);

	/*
	// Stripes
	lena64_gray = cv::imread("images/lena64_bars.png", CV_LOAD_IMAGE_GRAYSCALE);	// Load image as grayscale
	cv::imshow("Bars", lena64_gray);
	scaleMN = 1.0 / sqrt(lena64_gray.rows * lena64_gray.cols);
	lena64_gray.convertTo(lena64_gray, CV_64FC1, (1.0 / 255.0) * scaleMN);	// Scale image

	cv::Mat mask2(lena64_gray.rows, lena64_gray.cols, CV_8UC1, cv::Scalar(0));	// Black image for the mask
	int Xc = lena64_gray.cols / 2.0f;	// X coordinate of circle
	int Yc = lena64_gray.rows / 2.0f;	// Y coordinate of circle

	/*
	for (int i = 7; i < mask2.cols / 2; i+=7)
	{
		mask2.at<uchar>(Xc, Yc + i) = 255.0f;
		mask2.at<uchar>(Xc, Yc - i) = 255.0f;
	}
	
	int i = 28;
	mask2.at<uchar>(Xc, Yc + i) = 255.0f;
	mask2.at<uchar>(Xc, Yc - i) = 255.0f;

	cv::imshow("Maska2", mask2);

	complexMatrix = DiscreteFourierTransform(lena64_gray);
	ShowFourier(complexMatrix);

	ApplyMask(complexMatrix, mask2);

	restored = InvertedDiscreteFourierTransform(complexMatrix);
	cv::imshow("Restored bars", restored);
	cv::resizeWindow("Restored bars", 200, restored.rows);
	*/

	#pragma endregion DFT

	#pragma region Cviceni 6

	cv::Mat distorted = cv::imread("images/distorted_window.jpg", CV_LOAD_IMAGE_COLOR);
	//cv::Mat distorted = cv::imread("images/distorted_panorama.jpg", CV_LOAD_IMAGE_COLOR);
	//cv::imshow("Distorted", distorted);

	cv::Mat undistorted = Undistort(distorted, 0.2, 0.02);
	//cv::imshow("Undistorted", undistorted);

	#pragma endregion Lens distortion removal

	#pragma region Cviceni 7

	cv::Mat src_histogram_original = cv::imread("images/uneq.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	int histogram[256];

	cv::imshow("Original before equalization", src_histogram_original);

	GetHistogram(src_histogram_original, histogram);	// Histogram of original image
	ShowHistogram("Histogram image", histogram);

	EqualizeHistogram(src_histogram_original, histogram);

	cv::imshow("Equalized image", src_histogram_original);

	GetHistogram(src_histogram_original, histogram);	// Histogram of equalized image
	ShowHistogram("Equalized histogram", histogram);

	#pragma endregion Histogram equalization

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
