// HOGForDepth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Readvideo.h"
#include <string>
#include <atlstr.h>
#include <iostream>
#include <opencv2\opencv.hpp>
#include <direct.h>
#include <math.h>
#include <fstream>
using namespace std;
using namespace cv;

const int height = 480;
const int width = 640;
const double pi = 3.14159;
const double ep = 0.00001;
ofstream outfile;

void getAFrame(CvMat* cvMat, double &minDepth, double &maxDepth)
{
	CString videoFileName;
	videoFileName.Format("D:\\iData\\p08_01\\S08_000%d_1_0_20130412.oni",1);

	int x,y;
	ifstream depthFileReader;
	depthFileReader.open(videoFileName+"\\depth.dat",ios::binary);
	if(depthFileReader == NULL)
		return;

	ushort *depthData = new ushort[width*height];
	depthFileReader.read((char*)depthData,width*height*sizeof(ushort));
	if( !depthFileReader.fail() )
	{
		for(y=0; y<height; y++)
		{
			for(x=0; x<width; x++)
			{
				cvmSet( cvMat, y, x, (double)depthData[y*width+x]);
			}
		}
	}	
	delete [] depthData;

	minDepth = 5000;
	maxDepth = 0;
	//cvMinMaxLoc(cvMat,&minDepth,&maxDepth);
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			if (cvmGet( cvMat, y, x) > maxDepth)
			{
				maxDepth = cvmGet( cvMat, y, x);
			}
			if (cvmGet( cvMat, y, x) < minDepth && cvmGet( cvMat, y, x)>0)
			{
				minDepth = cvmGet( cvMat, y, x);
			}

		}
	}

// 	int index=0;
// 	while(index < 300)
// 	{
// 		index++;
// 		depthData = new ushort[width*height];
// 		depthFileReader.read((char*)depthData,width*height*sizeof(ushort));
// 		if( !depthFileReader.fail() )
// 		{
// 			for(y=0; y<height; y++)
// 			{
// 				for(x=0; x<width; x++)
// 				{
// 					cvmSet( cvMat, y, x, (double)depthData[y*width+x]);
// 				}
// 			}
// 		}	
// 		delete [] depthData;
// 
// 		cout<<cvmGet( cvMat, 240, 210)<<" ";
// 	}
}

void mat2Image(CvMat* cvMat, IplImage* image)
{
	double minDepth = 50000;
	double maxDepth = 0;
	//cvMinMaxLoc(cvMat,&minDepth,&maxDepth);
	for(int y=0; y<height; y++)
	{
		//cout<<cvmGet( cvMat, y, 100)<<" ";
		for(int x=0; x<width; x++)
		{
			if (cvmGet( cvMat, y, x) > maxDepth)
			{
				maxDepth = cvmGet( cvMat, y, x);
			}
			if (cvmGet( cvMat, y, x) < minDepth /*&& cvmGet( cvMat, y, x)>0*/)
			{
				minDepth = cvmGet( cvMat, y, x);
			}

		}
	}

	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			if (cvmGet( cvMat, y, x) == 0)
			{
				cvmSet( cvMat, y, x, 0);
			}
			else
			{
				double temp;
				temp = (cvmGet( cvMat, y, x) - minDepth)*255/(maxDepth - minDepth);
				cvmSet( cvMat, y, x, temp);
			}
		}
	}
	cvGetImage(cvMat,image);
	cvSaveImage("..\\output\\rice1.bmp",image);

}

void calGh(CvMat* cvMat_ori, CvMat* cvMat_Gh)
{
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			if (y==0 || x==0 || y==height-1 || x==width-1)
			{
				cvmSet( cvMat_Gh, y, x, 0);
			}
			else
			{
				//double temp = abs(cvmGet( cvMat_ori, y+1, x) - cvmGet( cvMat_ori, y-1, x));
				double temp = cvmGet( cvMat_ori, y+1, x) - cvmGet( cvMat_ori, y-1, x);
				cvmSet( cvMat_Gh, y, x, temp);
			}
		}
	}
}

void calGv(CvMat* cvMat_ori, CvMat* cvMat_Gv)
{
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			if (y==0 || x==0 || y==height-1 || x==width-1)
			{
				cvmSet( cvMat_Gv, y, x, 0);
			}
			else
			{
				//double temp = abs(cvmGet( cvMat_ori, y, x+1) - cvmGet( cvMat_ori, y, x-1));
				double temp = cvmGet( cvMat_ori, y, x+1) - cvmGet( cvMat_ori, y, x-1);
				cvmSet( cvMat_Gv, y, x, temp);
			}
		}
	}
}

void calMAndAngle(CvMat* cvMat_Gh, CvMat* cvMat_Gv, CvMat* cvMat_M, CvMat* cvMat_Angle)
{
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			//double temp = pow(cvmGet( cvMat_Gh, y, x),2) + pow(cvmGet( cvMat_Gv, y, x),2);
			double temp = abs(cvmGet( cvMat_Gh, y, x)) + abs(cvmGet( cvMat_Gv, y, x));
			cvmSet( cvMat_M, y, x, temp);
			double angleTemp;

			if (cvmGet( cvMat_Gv, y, x) == 0 && cvmGet( cvMat_Gh, y, x) > 0)
			{
				angleTemp = 1.57;
			}
			else if (cvmGet( cvMat_Gv, y, x) == 0 && cvmGet( cvMat_Gh, y, x) < 0)
			{
				angleTemp = -1.57;
			}
			else if (cvmGet( cvMat_Gv, y, x) == 0 && cvmGet( cvMat_Gh, y, x) == 0)
			{
				angleTemp = 0;
			}
			else
			{
				angleTemp = atan(cvmGet( cvMat_Gh, y, x)/cvmGet( cvMat_Gv, y, x));
			}
			cvmSet( cvMat_Angle, y, x, angleTemp);
		}
	}
}

void drawAngle(CvMat* cvMat_M, CvMat* cvMat_Angle)
{
	IplImage* img_show = cvCreateImage(cvSize(width,height),8,3);
	double scale = 0.1;
	int showThre = 10;   //length less than 10 will not be shown.
	for (int y=1; y<height; y+=4)
	{
		for (int x=1; x<width; x+=4)
		{
			CvPoint startP;
			startP.x = x;
			startP.y = y;
			//double length = cvmGet(cvMat_M, y, x)*scale;
			double length = 4.0;   //4.0 is used for showing. 
			if (abs(cvmGet(cvMat_M, y, x))<showThre)
			{
				length = 0.0;
			}
			double angle = cvmGet(cvMat_Angle, y, x) + pi/2;   //+1.57 if for drawing. The reason is unknown.
			CvPoint endP;
			endP.y = y + length*sin(angle);
			endP.x = x + length*cos(angle);

			cvLine(img_show,startP,endP,cvScalar(0,0,255),1,8,0);
		}
	}

	cvSaveImage("..\\output\\angle.bmp",img_show);
	cvReleaseImage(&img_show);
}

void calBlock(CvMat* cvMat_M, CvMat* cvMat_Angle, double* hogFeature,
	int startPointX, int startPointY, 
	int blockSize, int cellSize, int bin)
{
	int cellNum = blockSize/cellSize;
	int featurePoint = 0;
	int stepWidth = bin;
	for (int i=0; i<cellNum; i++)
	{
		for (int j=0; j<cellNum; j++)
		{
			CvMat* cvMat_M_cell = cvCreateMat(cellSize,cellSize,CV_64FC1);
			CvMat* cvMat_Angle_cell = cvCreateMat(cellSize,cellSize,CV_64FC1);
			for (int y=0; y<cellSize; y++)
			{
				for (int x=0; x<cellSize; x++)
				{
					double temp = cvmGet( cvMat_M, y+startPointY+i*cellSize, x+startPointX+j*cellSize);
					cvmSet( cvMat_M_cell, y, x, temp);
					temp = cvmGet( cvMat_Angle, y+startPointY+i*cellSize, x+startPointX+j*cellSize);
					cvmSet( cvMat_Angle_cell, y, x, temp);
				}
			}
			featurePoint = (i*cellNum + j)*bin;
			for (int k=featurePoint; k<featurePoint+bin; k++)
			{
				hogFeature[k] = 0.0;
			}
			for (int y=0; y<cellSize; y++)
			{
				for (int x=0; x<cellSize; x++)
				{
					double angle = cvmGet(cvMat_Angle_cell, y, x);
					double M = cvmGet(cvMat_M_cell,y, x);
					if (angle<0)
					{
						angle += pi;
					}

					int binNum = angle*bin/pi;
					hogFeature[featurePoint+binNum] += M;
				}
			}
			cvReleaseMat(&cvMat_M_cell);
			cvReleaseMat(&cvMat_Angle_cell);
		}
	}
}

void calHOG(CvMat* cvMat_M, CvMat* cvMat_Angle, double* hogFeature,
	int blockSize, int blockStrid, int cellSize, int bin) //blockSize should be divided by cellSize.
{
	int cellNum = blockSize/cellSize;
	int BlockNumH = (height-blockSize)/blockStrid + 1;
	int BlockNumW = (width-blockSize)/blockStrid + 1;
	int featurePoint = 0;
	for (int i=0; i<BlockNumH; i++)
	{
		for (int j=0; j<BlockNumW; j++)
		{
			featurePoint = (i*BlockNumW + j)*(bin*cellNum*cellNum);
			double* hog = new double[bin*cellNum*cellNum];
			calBlock(cvMat_M, cvMat_Angle, hog, j*blockStrid, i*blockStrid, blockSize, cellSize, bin);

			//Normalization in one block.
			double den = 0.0;
			//L2 norm
			for (int k=0; k<bin*cellNum*cellNum; k++)
			{
				den += hog[k]*hog[k];
			}
			den = sqrt(den + ep*ep);
			//L1 norm
// 			for (int k=0; k<bin*cellNum*cellNum; k++)
// 			{
// 				den += abs(hog[k]);
// 			}
// 			den = den + ep;
			for (int k=0; k<bin*cellNum*cellNum; k++)
			{
				hog[k] /= den;		
 				hogFeature[featurePoint + k] = hog[k];
				//outfile<<hogFeature[featurePoint + k]<<endl;
			}

// 			double sumHOG = 0.0;
// 			for (int k=0; k<bin*cellNum*cellNum; k++)
// 			{
// 				sumHOG += hog[k];
// 			}
// 			cout<<"total: "<<sumHOG<<endl;
			delete[] hog;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	CString s_filefolder;
	s_filefolder.Format("..\\output");
	_mkdir(s_filefolder);
	outfile.open("..\\output\\HOGfeature.csv", ios::out);

	CvMat* cvMat_ori = cvCreateMat(height,width,CV_64FC1);
	CvMat* cvMat_Gh = cvCreateMat(height,width,CV_64FC1);
	CvMat* cvMat_Gv = cvCreateMat(height,width,CV_64FC1);
	CvMat* cvMat_M = cvCreateMat(height,width,CV_64FC1);
	CvMat* cvMat_Angle = cvCreateMat(height,width,CV_64FC1);

	double minDepth;
	double maxDepth;
	getAFrame(cvMat_ori, minDepth, maxDepth); //Just get one frame for testing.
	cout<<minDepth<<'\t'<<maxDepth<<endl;

	IplImage* img_show = cvCreateImage(cvSize(width,height),8,1);
	//mat2Image(cvMat_ori,img_show);
	calGh(cvMat_ori, cvMat_Gh);
	calGv(cvMat_ori, cvMat_Gv);
	calMAndAngle(cvMat_Gh, cvMat_Gv, cvMat_M, cvMat_Angle);
	mat2Image(cvMat_M,img_show);
	drawAngle(cvMat_M, cvMat_Angle);


	int blockSize = 64;
	int blockStrid = 32;
	int cellSize = 32;
	int bin = 9;
	
	int cellNum = blockSize/cellSize;
	int BlockNumH = (height-blockSize)/blockStrid + 1;
	int BlockNumW = (width-blockSize)/blockStrid + 1;

	int dimension = BlockNumH*BlockNumW*bin*cellNum*cellNum;
	double* hogFeature = new double[dimension];
	calHOG(cvMat_M, cvMat_Angle, hogFeature, blockSize, blockStrid, cellSize, bin);

	//hogFeature is normalized here. The sum is 1.0;
	double den = 0.0;
	for (int k=0; k<dimension; k++)
	{
		den += abs(hogFeature[k]);
	}
	den = den+ep;
	for (int k=0; k<dimension; k++)
	{
		hogFeature[k] /= den;
		outfile<<hogFeature[k]<<endl;
	}



	delete[] hogFeature;
	cout<<"Done!"<<endl;
	outfile.close();
	getchar();
	return 0;
}

