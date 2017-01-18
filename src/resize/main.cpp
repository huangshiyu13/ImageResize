#include <opencv2/opencv.hpp>
#include <windows.h>
#include "FWH.h"
#include "preProcess.h"
#include "preProcessStrong.h"
#include <ctime>

#define RESIZE_DETECT_SIZE 10
const char *winTitle = "Resize";
double oriWHRadio;
IplImage *image,*oriImg,*copyImage;
FWH* fwhW;
FWH* fwhH;

CvSize winSize, imgSize,oriSize,minSize;

int resizeType = 0;
bool timeTest = false;


void cutW(IplImage*& image , CutPath path , CvSize size){
	//cout << "inW1"<<endl;
	int H = size.height;
	int W = size.width;
	//cout << "inW2"<<endl;
	//cout << "inW3"<<endl;
	for (int y = H - 1; y >= 0; --y) {
		if (path.size() != H){
			cout << "cutW() wrong:" << "path.size=" << path.size() << " " << "width:" << W  << "height:" << H << endl;
			return;
		} 
		//cout << y << endl;

		for (int x = path[H-1-y]; x < W - 1; ++x) {
			cvSet2D(image, y, x, cvGet2D(image,y,x+1));
		}
	}
	//cout << "inW4"<<endl;
}

void cutH(IplImage* &image , CutPath path , CvSize size){
	//cout << "inH "<<size.height << " " << size.width<<endl;
	int H = size.height;
	int W = size.width;
	//cout << "inH2"<<endl;

	//cout << "inH3"<<endl;
	for (int x = W - 1; x >= 0; --x) {
		if (path.size() != W){
			cout << "cutH() wrong:" << "path.size=" << path.size() << " " << "width:" << W  << "height:" << H << endl;
			return;
		} 

		//cout << x << " "<<path.size() << endl;
		for (int y = path[W-1-x]; y < H - 1; ++y) {
			//cout << x << " " << y <<endl;
			cvSet2D(image, y, x, cvGet2D(image,y+1,x));
		}
	}
	//cout << "inH4"<<endl;
}

void getImageH(int H , IplImage* &img){
	if (H == oriSize.height){
		return;
	}
	getImageH(H+1,img);
	cutH(img,fwhH[H].cutPath,cvSize(oriSize.width,H));
}

void getImageW(int W , IplImage* &img){
	//cout <<"w "<< W <<endl;
	if (W == oriSize.width){
		return;
	}
	getImageW(W+1,img);
	cutW(img,fwhW[W].cutPath,cvSize(W,oriSize.height));
}

void renderWindow(IplImage* img,  CvSize size){
	int H = size.height;
	int W = size.width;
	//cout << "img size:"<< img->height << " " << img->width << endl; 
	cvSet(image, CV_RGB(222, 222, 222));

	for (int h = 0 ; h < H-3 ; h ++){
		for (int w = 0 ; w < W-3 ; w++){
			cvSet2D(image, h, w, cvGet2D(img, h, w));
		}
	}
	cvShowImage(winTitle, image);
}

double getRadio(CvSize size){
	return double(size.width)/double(size.height);
}

IplImage* cvGetSubImage(IplImage *image, CvRect roi)
{
	IplImage *result;
	// ���� ROI 
	cvSetImageROI(image,roi);
	// ������ͼ��
	result = cvCreateImage( cvSize(roi.width, roi.height), image->depth, image->nChannels );
	cvCopy(image,result);
	cvResetImageROI(image);
	return result;
}

void getImage(CvSize size){
	double radioNow = getRadio(size);
	IplImage* desc;
	IplImage* getImage = cvCloneImage(oriImg);
	IplImage* pDest;
	CvRect roi;
	if (radioNow > oriWHRadio){
		int changeH = oriSize.width/radioNow;
		roi = cvRect(0,0,oriSize.width,changeH);
		getImageH(changeH,getImage);

	}
	else{
		int changeW = oriSize.height*radioNow;
		roi = cvRect(0,0,changeW,oriSize.height);
		getImageW(changeW,getImage);
	}

	//cout << "in0"<<endl;
	pDest = cvGetSubImage(getImage,roi);
	//cout << "in"<<endl;
	//cout << "imgg size "<<getImage->height << " " << getImage->width<<endl;
	desc = cvCreateImage(size,getImage->depth,getImage->nChannels);
	cvResize(pDest,desc);
	//cout << "in2"<<endl;
	//cvResize(oriImg,desc);
	renderWindow(desc,size);
	//cout << "in3"<<endl;
	cvReleaseImage(&getImage);
	cvReleaseImage(&desc);
	cvReleaseImage(&pDest);}

//��һ��ͼƬ
char* openFile() {
	//MAX_PATH = 260
	char szFileName[MAX_PATH] = {0};
	OPENFILENAME openFileName = {0};
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.nMaxFile = MAX_PATH;
	openFileName.lpstrFilter = "ͼƬ�ļ�(*.png;*.jpg;*.bmp)\0*.png;*.jpg;*.bmp\0�����ļ�(*.*)\0*.*\0\0";
	openFileName.lpstrFile = szFileName;
	openFileName.nFilterIndex = 1;
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!::GetOpenFileName(&openFileName)) return NULL;
	char *fileName = new char[999];
	strcpy(fileName, openFileName.lpstrFile);
	return fileName;
}

void createWindow(const char *filename) {
	IplImage *imageLoad = cvLoadImage(filename);
	winSize.width = (imgSize.width = imageLoad->width)*1.1;
	winSize.height = (imgSize.height = imageLoad->height)*1.1;
	oriSize.width = imgSize.width;
	oriSize.height = imgSize.height;
	oriWHRadio = double(oriSize.width)/double(oriSize.height);
	image = cvCreateImage(winSize, IPL_DEPTH_8U, 3);
	cvSet(image, CV_RGB(222, 222, 222));

	for (int x = 0; x < imageLoad->width; ++x)
		for (int y = 0; y < imageLoad->height; ++y)
			cvSet2D(image, y, x, cvGet2D(imageLoad, y, x));

	minSize = cvSize(int(imgSize.width*0.8),int(imgSize.height*0.8));
	fwhH = new FWH[imgSize.height+1];
	fwhW = new FWH[imgSize.width+1];
	IplImage* copyImg = cvCloneImage(imageLoad);
	oriImg = cvCloneImage(imageLoad);
	int op,op2;
	cout << "ѡ������ģʽ(1 for ����ģʽ , 2 for ǿ��ģʽ):";
	cin >> op;
	//cout << "�Ƿ����ü�ʱ����?(1 for �������� , 2 for �ݲ�����):";
	//cin >> op2;
	if(op2 == 1){
		timeTest = true;
	}
	if (op == 1){
		preProcessH(fwhH, copyImg , imgSize);
		preProcessW(fwhW, copyImg , imgSize);
	}
	else{
		preProcessHS(fwhH, copyImg , imgSize);
		preProcessWS(fwhW, copyImg , imgSize);
	}
	cvNamedWindow(winTitle);
	cvReleaseImage(&imageLoad);
	cvShowImage(winTitle, image);
}
void createWindow(const char *filename, int sp) {
	cvNamedWindow(winTitle);
	image = cvLoadImage(filename);
	cvShowImage(winTitle, image);
}

void destroyWindow() {
	cvDestroyWindow(winTitle);
	cvReleaseImage(&image);
}

void showWLine(int W, int H){
	for (int i = 0 ; i < W ; i++){
		if (i%8 < 5){
			cvSet2D(copyImage, H-3, i, CV_RGB(255, 255, 0));
			cvSet2D(copyImage, H-2, i, CV_RGB(255, 255, 0));
			cvSet2D(copyImage, H-1, i, CV_RGB(255, 255, 0));
		}
	}
}

void showHLine(int H, int W){
	for (int i = 0 ; i < H ; i++){
		if (i%8 < 5){
			cvSet2D(copyImage, i, W-3, CV_RGB(255, 255, 0));
			cvSet2D(copyImage, i, W-2, CV_RGB(255, 255, 0));
			cvSet2D(copyImage, i, W-1, CV_RGB(255, 255, 0));
		}
	}
}

void showLine(CvSize size){
	cvReleaseImage(&copyImage);
	copyImage = cvCloneImage(image);
	showWLine(size.width,size.height);
	showHLine(size.height,size.width);
	cvShowImage(winTitle, copyImage);

}
void onMouse(int Event, int x, int y, int flags, void *param ) {
	if (Event == CV_EVENT_LBUTTONDOWN) {
		if (abs(x - imgSize.width) < RESIZE_DETECT_SIZE) {
			resizeType = 1;
		}
		if (abs(y - imgSize.height) < RESIZE_DETECT_SIZE) {
			resizeType = 2;
		}
		if (abs(x - imgSize.width) < RESIZE_DETECT_SIZE && abs(y - imgSize.height) < RESIZE_DETECT_SIZE){
			resizeType = 3;
		}
	}
	if (Event == CV_EVENT_MOUSEMOVE && resizeType != 0){
		if (resizeType == 1) {
			imgSize.width = x>winSize.width?winSize.width:x;
			imgSize.width = imgSize.width<1?1:imgSize.width;
		}
		if (resizeType == 2) {
			imgSize.height = y>winSize.height?winSize.height:y;
			imgSize.height = imgSize.height<1?1:imgSize.height;
		}
		if (resizeType == 3){
			imgSize.width = x>winSize.width?winSize.width:x;
			imgSize.width = imgSize.width<1?1:imgSize.width;
			imgSize.height = y>winSize.height?winSize.height:y;
			imgSize.height = imgSize.height<1?1:imgSize.height;
			
		}
		showLine(imgSize);
	}
	
	if (Event == CV_EVENT_LBUTTONUP && resizeType != 0) {
		getImage(imgSize);
		cout <<"�߶�:"<< imgSize.height << "px ���:"<<imgSize.width<<"px"<<endl;
		resizeType = 0;
	}
}

int main() {
	if (::MessageBoxA(NULL, "��ѡ��һ��ͼƬ", "����", 0x00000001L) == 1){

		char *fileName;
		if ( (fileName = openFile()) == NULL) return 0;

		createWindow(fileName);

		cvSetMouseCallback(winTitle, onMouse);
		while((cvWaitKey(10)&0xff) != 27);//ESC
		destroyWindow();
	}
	return 0;
}