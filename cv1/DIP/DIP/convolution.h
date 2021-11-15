template <typename T, unsigned int maskDim>
void Convolution(cv::Mat& original, cv::Mat& resultImg, double mask[maskDim][maskDim], double scale = 1.0)
{
	original.copyTo(resultImg);

	int border = maskDim / 2;
	for (int y = border; y < original.rows - border; y++)
	{
		for (int x = border; x < original.cols - border; x++)
		{
			T result = 0;
			for (int i = -border; i < (border + 1); i++)
			{
				for (int j = -border; j < (border + 1); j++)
				{
					T pix = original.at<T>(y + i, x + j);
					pix *= mask[(i + border)][(j + border)];
					result += pix;
				}
			}

			result /= scale;
			resultImg.at<T>(y, x) = result;
		}
	}
}

/*
cv::Mat Convolution(cv::Mat original, cv::Mat mask, float coeficient)
{
	cv::Mat result;
	original.copyTo(result);

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
*/
