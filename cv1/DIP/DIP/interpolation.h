#include "stdafx.h"

static double GetIntegral(double n)
{
	double fr;	// Only for modf(), does nothing
	return modf(n, &fr);
}

template <typename T> T bilinearInterpolation(cv::Mat& image, double x, double y)
{
	cv::Point2d directions[4];
	double coeficients[4];

	// Directions
	directions[0] = cv::Point2d(std::floor(x), std::floor(y));
	directions[1] = cv::Point2d(std::floor(x), std::ceil(y));
	directions[2] = cv::Point2d(std::ceil(x), std::floor(y));
	directions[3] = cv::Point2d(std::ceil(x), std::ceil(y));

	// Counting coeficients
	coeficients[0] = (1 - GetIntegral(x)) * (1 - GetIntegral(y));
	coeficients[1] = (1 - GetIntegral(x)) * (GetIntegral(y));
	coeficients[2] = (GetIntegral(x)) * (1 - GetIntegral(y));
	coeficients[3] = (GetIntegral(x)) * (GetIntegral(y));

	T result;	// Result of interpolation

	for (int i = 0; i < 4; i++)
	{
		cv::Point2d p = directions[i];
		if (p.x >= 0 && p.x < image.rows && 
			p.y >= 0 && p.y < image.cols)
		{
			result += coeficients[i] * image.at<T>((int)(p.x), (int)(p.y));
		}
	}

	return result;
}