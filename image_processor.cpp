#include <QApplication>
#include <QPainter>
#include <QImageReader>
#include "raster/deskewer.h"
#include <math.h>
#include "image_processor.h"
#include <iostream>

ImageProcessor::ImageProcessor() {}

int ImageProcessor::sumIndexes(QImage image,int linenum) {
	int result=0;
	for (int i=0;i<image.width(); i++) {
		result+=image.pixelIndex(i,linenum);
	}		
	return result;
}




bool ImageProcessor::isBlackOne(QImage image) {
	return sumIndexes(image,0)<image.width()/2;
}


QImage* ImageProcessor::deskew(QImage image) {

	double alpha =0;
	QImage monoImage=image.convertToFormat(QImage::Format_Mono);
	//	monoImage.save("testew.png","png");
	alpha = pagetools::Deskewer::findSkew(monoImage);
	QImage* resImage=new QImage((int)(image.width()*cos(alpha)+image.height()*sin(alpha)),(int)(image.width()*sin(alpha)+image.height()*cos(alpha)),QImage::Format_RGB32);
	//	std::cout << " alpha:" << alpha*180*M_1_PI << std::endl;
	QPainter paint(resImage);
	paint.fillRect(0,0,resImage->width(),resImage->height(),QBrush(Qt::white));
	paint.rotate(alpha*180/M_PI);
	paint.drawImage(0,0,image);
	//	resImage->save("test.png","png");


	return resImage;
}



void ImageProcessor::countLine(QImage image,int linenum, LineInfo *lineInfo) {
	int i=0;
	int col=0;
	lineInfo->sum=0;
	lineInfo->left=0;
	lineInfo->right=0;
	if (blackIsOne) {
		for (;(i<image.width() && (col=image.pixelIndex(i,linenum))!=1);i++) ;
		lineInfo->left=i;
		lineInfo->sum=0;
		int ni=0;
		for (;i<image.width(); i++) {
			ni=image.pixelIndex(i,linenum);
			lineInfo->sum+=ni;
			if (ni!=0) lineInfo->right=i;
		}		
	}else {
		for (;(i<image.width() && (col=image.pixelIndex(i,linenum))!=0);i++) ;
		lineInfo->left=i;
		lineInfo->sum=0;
		int ni=0;
		for (;i<image.width(); i++) {
			ni=image.pixelIndex(i,linenum);
			lineInfo->sum+=1-ni;
			if (ni!=1) lineInfo->right=i;
		}		
	}
	//	std::cout << " left:" << lineInfo->left << " right:" << lineInfo->right << " sum:" << lineInfo->sum <<std::endl;
}

bool ImageProcessor::isLineEmpty(ImageProcessor::LineInfo info,int mean) {
	return (info.sum<mean/10);// && (info.sum<10);
}

bool ImageProcessor::isLineBlank(ImageProcessor::LineInfo info) {
	return info.sum==0;
}
int copyCurrentLine(int leftmost,int rightmost,int i,QPainter *paint,QImage *image, QImage *resImage,int resHeight, int numPrevNonEmptyLines, int width) {

	int rh=resHeight;
	int j=leftmost;
	do {
		paint->drawImage(0,rh,image->copy(j,i-numPrevNonEmptyLines,width,numPrevNonEmptyLines));
		rh+=numPrevNonEmptyLines;
		j+=width; 
	} while (j<(rightmost-width)); 
	paint->drawImage(0,rh,image->copy(j,i-numPrevNonEmptyLines,rightmost-j,numPrevNonEmptyLines));
	rh+=numPrevNonEmptyLines;
	return rh;
}

bool ImageProcessor::isLineText(ImageProcessor::LineInfo info,int mean) {
	return info.sum>mean/3;
}

QList<QImage> * ImageProcessor::processImage(QString filename) {
	QList<QImage> * resList = new QList<QImage>();
	QImage image= QImage(filename);
	QImage  * tmpim=deskew(image);
	image=QImage(*tmpim);
	QImage monoImage=image.convertToFormat(QImage::Format_Mono);
	delete tmpim;
	monoImage.save("test2.png","png");
	//	image.save("test3.png","png");
	blackIsOne=isBlackOne(monoImage);
	//char *prefix="resized_";
	int numPrevNonEmptyLines=0;
	int resHeight=0;
	QImage resImage(width,(monoImage.height()*monoImage.width())/width+1,QImage::Format_RGB32);
	QPainter paint(&resImage);
	paint.fillRect(0,0,width,(monoImage.height()*monoImage.width())/width+1,QBrush(Qt::white));


	long int mean=0;
	int leftmost=monoImage.width();
	int rightmost=0;
	int nonEmptyNum=0;
	ImageProcessor::LineInfo lineInfos[monoImage.height()];
	for (int i=0;i<monoImage.height();i++) {
		countLine(monoImage,i,&lineInfos[i]);
		mean+=lineInfos[i].sum;
		if (lineInfos[i].sum>0) {
			if (leftmost>lineInfos[i].left)  leftmost=lineInfos[i].left;
			if (rightmost<lineInfos[i].right) rightmost=lineInfos[i].right;
			nonEmptyNum++;
		}
	}
	mean=mean/nonEmptyNum;
	std::cout << "lm: " << leftmost << " rm: " << rightmost << " mean: " << mean <<  " ne num " << nonEmptyNum << std::endl;

	bool readyToCut = false;
	int cutLine=0;
	int cutLineSum=mean;
	int cutSize=0;
	for (int i=0; i<monoImage.height(); i++ ) {
		if (isLineEmpty(lineInfos[i],mean)) {
			if (numPrevNonEmptyLines>8) {
				readyToCut=true;
				numPrevNonEmptyLines ++; 
				if (lineInfos[i].sum<cutLineSum) {
					cutLine=i;
					cutSize=numPrevNonEmptyLines;
				} 
				if (isLineBlank(lineInfos[i])) {
					resHeight=copyCurrentLine(leftmost,rightmost,cutLine,&paint,&image,&resImage,resHeight,cutSize,width);
					numPrevNonEmptyLines-=cutSize;
					readyToCut=false;
					cutLineSum=mean;
				}
			} else if ((numPrevNonEmptyLines>0) || (!isLineBlank(lineInfos[i]))){ 
				numPrevNonEmptyLines++;
			}
		} else {
			numPrevNonEmptyLines++;
			if (isLineText(lineInfos[i],mean) && readyToCut) {
				resHeight=copyCurrentLine(leftmost,rightmost,cutLine,&paint,&image,&resImage,resHeight,cutSize,width);
				numPrevNonEmptyLines-=cutSize;
				readyToCut=false;
				cutLineSum=mean;
			}
		}
	}
	if (numPrevNonEmptyLines>0) {
		resHeight=copyCurrentLine(leftmost,rightmost,monoImage.height(),&paint,&image,&resImage,resHeight,numPrevNonEmptyLines,width);
	}
//	resImage.save("test4.png","png");
	QImage toWrite = resImage.copy(0,0,width,resHeight);
	//toWrite.convertToFormat(QImage::Format_Mono);
	//		char *resFileName = new char[500];
	//		strcpy(resFileName,prefix);
	//		strcat(resFileName,fname);
	//		toWrite.save(resFileName,"png");
	resList->append(toWrite);
	return resList;
}



