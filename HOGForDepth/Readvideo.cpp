#include "StdAfx.h"
#include "Readvideo.h"


Readvideo::Readvideo(void)
{
}


Readvideo::~Readvideo(void)
{
	vSkeletonData.clear();

	for(int i=0;i<vDepthData.size();i++)
		vDepthData[i].release();
	vDepthData.clear();

	for(int i=0;i<vColorData.size();i++)
		cvReleaseImage(&(vColorData[i]));
	vColorData.clear();
}

void Readvideo::readvideo(string filePath)
{
	clock_t start, durTime;
	start=clock();
	if( !( readColorFrame(filePath) && readDepthFrame(filePath) && readSkeletonFrame(filePath) ))
	{
		return;
	}	

	if( !( vSkeletonFrame.size() == vColorFrame.size() && vSkeletonFrame.size() == vDepthFrame.size() ))
	{
		return;
	}

	string str = filePath.substr(filePath.length()-25,21);
	cout<<str<<endl;
	cout<<"Reading..."<<endl;
	const char* savefilename=str.c_str();

	//读取彩色、深度和骨架数据
	const char* filePathChar=filePath.c_str();
	char filePathName[100];
	strcpy(filePathName,filePathChar);
	strcat(filePathName,"\\color.avi");
	CvCapture *capture = cvCreateFileCapture(filePathName);
	if( NULL == capture )
		return;

	int x,y;
	ifstream depthFileReader;
	depthFileReader.open(filePath+"\\depth.dat",ios::binary);
	if(depthFileReader == NULL)
		return;

	ifstream skeletonFileReader;
	skeletonFileReader.open(filePath+"\\skeleton.dat",ios::binary);
	if(skeletonFileReader == NULL)
		return;

	//read first color depth and skeleton data
	IplImage *frame = cvQueryFrame(capture);
	vColorData.push_back(cvCloneImage(frame));

	Mat depthMat;
	depthMat.create(480,640,CV_16UC1);
	ushort *depthData = new ushort[640*480];
	depthFileReader.read((char*)depthData,640*480*sizeof(ushort));
	if( !depthFileReader.fail() )
	{
		for(y=0; y<480; y++)
		{
			for(x=0; x<640; x++)
			{
				depthMat.at<ushort>(y,x) = depthData[y*640+x];
			}
		}
	}	
	delete [] depthData;
	vDepthData.push_back(depthMat.clone());

	SLR_ST_Skeleton mSkeleton;
	skeletonFileReader.read((char*)&mSkeleton,sizeof(mSkeleton));
	

	SLR_ST_Skeleton sLast = mSkeleton;
	SLR_ST_Skeleton sCurrent;
	int index = 0;

	vSkeletonData.clear();
	vSkeletonData.push_back(mSkeleton);
	
	int stillCount = 0;

	while(index < vSkeletonFrame.size()-1)
	{
		//读取下一帧的彩色、深度和骨架数据
		index++;
		frame = cvQueryFrame(capture);
		vColorData.push_back(cvCloneImage(frame));
		
		depthData = new ushort[640*480];
		depthFileReader.read((char*)depthData,640*480*sizeof(ushort));
		if( !depthFileReader.fail() )
		{
			for(y=0; y<480; y++)
			{
				for(x=0; x<640; x++)
				{
					depthMat.at<ushort>(y,x) = depthData[y*640+x];
				}
			}
		}	
		delete [] depthData;
		vDepthData.push_back(depthMat.clone());
		skeletonFileReader.read((char*)&sCurrent,sizeof(sCurrent));

		//if begin, judge end time and do hand sgementation
		vSkeletonData.push_back(sCurrent);
		sLast = sCurrent;

	}


	durTime=clock()-start;
	cout<<"Read Data Time:	"<<durTime<<endl;

	start=clock();

	depthFileReader.close();
	skeletonFileReader.close();
	cvReleaseCapture(&capture);
}

bool Readvideo::readColorFrame(string filename)
{
	vColorFrame.clear();
	ifstream colorFrameReader;
	const char* filePathChar=filename.c_str();
	char filePathName[100];
	strcpy(filePathName,filePathChar);
	strcat(filePathName,"\\color.frame");
	colorFrameReader.open(filePathName,ios::binary);
	if(colorFrameReader == NULL)
		return false;
	while( !colorFrameReader.eof() )
	{
		LONGLONG colorframeno;
		colorFrameReader.read((char*)&colorframeno,sizeof(LONGLONG));
		if( colorFrameReader.fail() )
			break;
		vColorFrame.push_back(colorframeno);
	}
	colorFrameReader.close();
	return true;
}

bool Readvideo::readDepthFrame(string filename)
{
	vDepthFrame.clear();
	ifstream depthFrameReader;
	const char* filePathChar=filename.c_str();
	char filePathName[100];
	strcpy(filePathName,filePathChar);
	strcat(filePathName,"\\depth.frame");
	depthFrameReader.open(filePathName,ios::binary);
	if(depthFrameReader == NULL)
		return false;
	while( !depthFrameReader.eof() )
	{
		LONGLONG depthframeno;
		depthFrameReader.read((char*)&depthframeno,sizeof(LONGLONG));
		if( depthFrameReader.fail() )
			break;
		vDepthFrame.push_back(depthframeno);
	}
	depthFrameReader.close();
	return true;
}

bool Readvideo::readSkeletonFrame(string filename)
{
	vSkeletonFrame.clear();
	ifstream skeletonFrameReader;
	const char* filePathChar=filename.c_str();
	char filePathName[100];
	strcpy(filePathName,filePathChar);
	strcat(filePathName,"\\skeleton.frame");
	skeletonFrameReader.open(filePathName,ios::binary);
	if(skeletonFrameReader == NULL)
		return false;
	while(!skeletonFrameReader.eof())
	{
		LONGLONG skeletonframeno;
		skeletonFrameReader.read((char*)&skeletonframeno,sizeof(LONGLONG));
		if( skeletonFrameReader.fail() )
			break;
		vSkeletonFrame.push_back(skeletonframeno);
	}
	skeletonFrameReader.close();
	return true;
}
