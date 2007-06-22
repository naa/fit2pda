#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H


#include <QList>
#include <QImage>

class ImageProcessor {
		private:
				typedef struct {
						int sum;
						int left;
						int right;
				} LineInfo;

				int sumIndexes(QImage image,int linenum); 
				bool isBlackOne(QImage image); 
				QImage* deskew(QImage image); 
				void countLine(QImage image,int linenum, LineInfo *lineInfo); 

				bool isLineEmpty(LineInfo info,int mean);
				bool isLineText(LineInfo info,int mean);
				bool isLineBlank(LineInfo info);
				bool blackIsOne;

		public:
				int width;
				int height;


				ImageProcessor();

				QList<QImage> * processImage(QString filename);
};

#endif
