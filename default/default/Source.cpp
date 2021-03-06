#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "opencv2/imgcodecs.hpp"


using namespace cv;
using namespace std;

void drawStuff();
void drawAllTriangles(Mat&, const vector< vector<Point> >&);

Mat img_rgb, img_gray, canny_output, drawing;

int thresht = 100;


int saveimg();
static void help()
{
	cout << "\nThis program demonstrates circle finding with the Hough transform.\n"
		"Usage:\n"
		"./houghcircles <image_name>, Default is ../data/board.jpg\n" << endl;
}

int thresh = 50, N = 5;
const char* wndname = "Detected square";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares(const Mat& image, vector<vector<Point> >& squares)
{
	squares.clear();

	//s    Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	// down-scale and upscale the image to filter out the noise
	//pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
	//pyrUp(pyr, timg, image.size());


	// blur will enhance edge detection
	Mat timg(image);
	medianBlur(image, timg, 9);
	Mat gray0(timg.size(), CV_8U), gray;

	vector<vector<Point> > contours;

	// find squares in every color plane of the image
	for (int c = 0; c < 3; c++)
	{
		int ch[] = { c, 0 };
		mixChannels(&timg, 1, &gray0, 1, ch, 1);

		// try several threshold levels
		for (int l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if (l == 0)
			{
				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				Canny(gray0, gray, 5, thresh, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1, -1));
			}
			else
			{
				// apply threshold if l!=0:
				//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
				gray = gray0 >= (l + 1) * 255 / N;
			}

			// find contours and store them all as a list
			findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

			vector<Point> approx;

			// test each contour
			for (size_t i = 0; i < contours.size(); i++)
			{
				// approximate contour with accuracy proportional
				// to the contour perimeter
				approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

				// square contours should have 4 vertices after approximation
				// relatively large area (to filter out noisy contours)
				// and be convex.
				// Note: absolute value of an area is used because
				// area may be positive or negative - in accordance with the
				// contour orientation
				if (approx.size() == 4 &&
					fabs(contourArea(Mat(approx))) > 1000 &&
					isContourConvex(Mat(approx)))
				{
					double maxCosine = 0;

					for (int j = 2; j < 5; j++)
					{
						// find the maximum cosine of the angle between joint edges
						double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
						maxCosine = MAX(maxCosine, cosine);
					}

					// if cosines of all angles are small
					// (all angles are ~90 degree) then write quandrange
					// vertices to resultant sequence
					if (maxCosine < 0.3)
						squares.push_back(approx);
				}
			}
		}
	}
}


// the function draws all the squares in the image
static void drawSquares(Mat& image, const vector<vector<Point> >& squares)
{
	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];

		int n = (int)squares[i].size();
		//dont detect the border
		if (p->x > 3 && p->y > 3)
			polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	imshow(wndname, image);
}

int main(int argc, char** argv)
{
	saveimg();
	
	cv::CommandLineParser parser(argc, argv,
		"{help h ||}{@image|../data/board.jpg|}"
	);
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	string filename = parser.get<string>("@image");
	if (filename.empty())
	{
		help();
		cout << "no image_name provided" << endl;
		return -1;
	}

	Mat img = imread("img/frame8.jpg", 0);
	if (img.empty())
	{
		help();
		cout << "can not open " << filename << endl;
		return -1;
	}
	Mat cimg;
	medianBlur(img, img, 5);
	cvtColor(img, cimg, COLOR_GRAY2BGR);
	vector<Vec3f> circles;
	HoughCircles(img, circles, HOUGH_GRADIENT, 1, 10,
		100, 30, 1, 100 // change the last two parameters
					   // (min_radius & max_radius) to detect larger circles
	);
	for (size_t i = 0; i < circles.size(); i++)
	{
		Vec3i c = circles[i];
		circle(cimg, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 3, LINE_AA);
		circle(cimg, Point(c[0], c[1]), 2, Scalar(0, 255, 0), 3, LINE_AA);
	}
	
	///////////////////////////////////////////////////////////////////////////////////
	
	vector<vector<Point> > squares;
	static const char* names[] = { "img/frame8.jpg",0 };
	for (int i = 0; names[i] != 0; i++)
	{
		Mat image = imread(names[i], 1);
		if (image.empty())
		{
			cout << "Couldn't load " << names[i] << endl;
			continue;
		}

		findSquares(image, squares);
		drawSquares(image, squares);
		//imwrite( "out", image );
		int c = waitKey();
		if ((char)c == 27)
			break;
	}
	imshow("detected", cimg);
	waitKey();

	img_rgb = imread("img/frame8.jpg");
	cvtColor(img_rgb, img_gray, CV_RGB2GRAY);
	//imshow("InputImage", img_rgb);
	drawStuff();
	waitKey();
	return 0;
}

int readimg(int no) {
	int i = 0;
	while (true)
	{
		Mat imgOriginal = imread("img/frame" + std::to_string(i) + ".jpg");
		i++;
		imshow("Original", imgOriginal); //show the original image
		imwrite("img/frame" + std::to_string(no) + ".jpg", imgOriginal);
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
		if (i == no)
		{
			return 0;
		}

	}
	return 0;
}



int saveimg() {
	VideoCapture cap(0); //capture the video from web cam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}
	int no = 0;
	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}
		imshow("Original", imgOriginal); //show the original image
		imwrite("img/frame" + std::to_string(no) + ".jpg", imgOriginal);
		no++;
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}

	}
}
	

	int imageread(int argc, char** argv)
	{
		cv::CommandLineParser parser(argc, argv,
			"{help h ||}{@image|../data/board.jpg|}"
		);
		if (parser.has("help"))
		{
			help();
			return 0;
		}
		string filename = parser.get<string>("@image");
		if (filename.empty())
		{
			help();
			cout << "no image_name provided" << endl;
			return -1;
		}
		Mat img = imread(filename, 0);
		if (img.empty())
		{
			help();
			cout << "can not open " << filename << endl;
			return -1;
		}
		Mat cimg;
		medianBlur(img, img, 5);
		cvtColor(img, cimg, COLOR_GRAY2BGR);
		vector<Vec3f> circles;
		HoughCircles(img, circles, HOUGH_GRADIENT, 1, 10,
			100, 30, 1, 30 // change the last two parameters
						   // (min_radius & max_radius) to detect larger circles
		);
		for (size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			circle(cimg, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 3, LINE_AA);
			circle(cimg, Point(c[0], c[1]), 2, Scalar(0, 255, 0), 3, LINE_AA);
		}
		imshow("Detected Circle", cimg);
		waitKey();
		return 0;
	}

	void drawStuff() {
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		
		Canny(img_gray, canny_output, thresht, thresht * 2, 3);
		//imshow("Canny", canny_output);
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		drawing = Mat::zeros(canny_output.size(), CV_8UC3);

		drawAllTriangles(drawing, contours);
		imshow("Detected tringle", drawing);
	}

	void drawAllTriangles(Mat& img, const vector< vector<Point> >& contours) {
		vector<Point> approxTriangle;
		for (size_t i = 0; i < contours.size(); i++) {
			approxPolyDP(contours[i], approxTriangle, arcLength(Mat(contours[i]), true)*0.05, true);
			if (approxTriangle.size() == 3) {
				drawContours(img, contours, i, Scalar(0, 255, 255), CV_FILLED); // fill GREEN
				vector<Point>::iterator vertex;
				for (vertex = approxTriangle.begin(); vertex != approxTriangle.end(); ++vertex) {
					circle(img, *vertex, 3, Scalar(0, 0, 255), 1);
				}
			}
		}
	}
	
