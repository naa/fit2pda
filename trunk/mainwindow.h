#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "image_processor.h"
#include <QFileInfo>
#include <QDir>

class QAction;
class QMenu;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();
		public slots:		
			void changeWidth(int newWidth);
		void changeDpi(int newDpi);
		void changeChunkSize(int chunkSize);
		void selectOutputDir();
	protected:
		void closeEvent(QCloseEvent *event);

		private slots:
			void addFile();
		void addDir();
		void clear();
		void convert();
		void about();

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void addFile(const QString &fileName);
		int psToPng(QFileInfo finfo, int filenum, int filecount);
		int djvuToPs(QFileInfo finfo, int filenum, int filecount);

		int pngToPng(QDir inpDir, QDir outDir, int filenum, int filecount);


		QListWidget* fileList;
		QLineEdit *outEdit;
		QList<QString> *fileNamesList;

		ImageProcessor *imageProcessor;	

		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *helpMenu;
		QToolBar *fileToolBar;

		QAction *addAct;
		QAction *clearAct;
		QAction *convertAct;
		QAction *exitAct;
		QAction *aboutAct;
		QAction *aboutQtAct;
		QAction *addDirAct;

		int dpi;
		int chunkSize;
		QString outputDir;
};

#endif
