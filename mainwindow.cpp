#include <QtGui>
#include "mainwindow.h"
#include <iostream>
MainWindow::MainWindow() {
  outputDir=QDir::current().canonicalPath();

  QWidget *mainWidget = new QWidget(this);
  QGroupBox *inputBox = new QGroupBox(tr("Input files"),mainWidget);
  QVBoxLayout *vbl=new QVBoxLayout;
  fileList=new QListWidget;
  vbl->addWidget(fileList);
  inputBox->setLayout(vbl);

  QGroupBox *outputBox = new QGroupBox(tr("Output directory"),mainWidget);
  QHBoxLayout *hbl=new QHBoxLayout;
  outEdit=new QLineEdit;
  outEdit->setText(outputDir);
  outEdit->setReadOnly(true);
  QPushButton *selectButton=new QPushButton("&Select");
  connect(selectButton, SIGNAL(clicked()),
	  this, SLOT(selectOutputDir()));
  hbl->addWidget(outEdit);
  hbl->addWidget(selectButton);
  outputBox->setLayout(hbl);


  QGroupBox *groupBox = new QGroupBox(tr("Conversion settings"),mainWidget);
  QVBoxLayout *vbox = new QVBoxLayout;
  QLabel *widthLabel = new QLabel(tr("Enter desired width between "
				     "%1 and %2:").arg(20).arg(1000));
  QSpinBox *widthSpinBox = new QSpinBox;
  widthSpinBox->setRange(20, 1000);
  widthSpinBox->setSingleStep(1);
  widthSpinBox->setValue(300);
  connect(widthSpinBox, SIGNAL(valueChanged(int)),
	  this, SLOT(changeWidth(int)));

  QLabel *dpiLabel = new QLabel(tr("Enter desired resolution (DPI) between "
				   "%1 and %2:").arg(50).arg(600));
  QSpinBox *dpiSpinBox = new QSpinBox;
  dpiSpinBox->setRange(50, 600);
  dpiSpinBox->setSingleStep(1);
  dpiSpinBox->setValue(170);
  dpi=170;
  connect(dpiSpinBox, SIGNAL(valueChanged(int)),
	  this, SLOT(changeDpi(int)));

  QLabel *chunkLabel = new QLabel(tr("Enter desired document chunk size in pages between "
				     "%1 and %2:").arg(1).arg(5000));
  QSpinBox *chunkSpinBox = new QSpinBox;
  chunkSpinBox->setRange(1, 5000);
  chunkSpinBox->setSingleStep(1);
  chunkSpinBox->setValue(5000);
  chunkSize=5000;
  connect(chunkSpinBox, SIGNAL(valueChanged(int)),
	  this, SLOT(changeChunkSize(int)));

  vbox->addWidget(widthLabel);
  vbox->addWidget(widthSpinBox);
  vbox->addWidget(dpiLabel);
  vbox->addWidget(dpiSpinBox);
  vbox->addWidget(chunkLabel);
  vbox->addWidget(chunkSpinBox);
  groupBox->setLayout(vbox);

  QVBoxLayout *mainVBox = new QVBoxLayout;
  mainVBox->addWidget(inputBox);
  mainVBox->addWidget(outputBox);
  mainVBox->addWidget(groupBox);
  mainWidget->setLayout(mainVBox);
  setCentralWidget(mainWidget);
  imageProcessor=new ImageProcessor();
  imageProcessor->width=300;
  fileNamesList = new QList<QString>;
  createActions();
  createMenus();
  createToolBars();
  createStatusBar();

  setWindowTitle(tr("Fit to PDA"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  event->accept();
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About Application"),
		     tr("The <b>Fit to PDA</b> converts scanned books "
			"to fit to small PDA or Smartphone screen"));
}

void MainWindow::addFile() {
  QString s = QFileDialog::getOpenFileName(
					   this,
					   "Choose a file",
					   ".",
					   "Books (*.pdf *.ps *.djvu)");
  if (!s.isEmpty()) {
    fileNamesList->append(s);
    new QListWidgetItem(s, fileList);
  }
}

void MainWindow::addDir() {
  QString s = QFileDialog::getExistingDirectory (
						 this,
						 "Choose directory with png images",
						 ".");
  if (!s.isEmpty()) {
    fileNamesList->append(s);
    new QListWidgetItem(s, fileList);
  }
}


void MainWindow::clear() {
  fileList->clear();
  fileNamesList->clear();
}

int MainWindow::psToPng(QFileInfo finfo,int filenum,int filecount) {
  //	QString cmd=QString("gs -dBATCH -dQUIET -sDEVICE=png16m -r")+QString::number(dpi)+QString(" -sOutputFile=\"");
  //	cmd+=outDir.canonicalPath();
  //	cmd+="/%05d.png\" \""+finfo.canonicalFilePath()+"\"";
  //	std::cout << "pstopng " << cmd.toStdString() << std::endl;
  //	int status=system(cmd.toAscii());
  //	return status;
  QDir outDir(outputDir);
  outDir.mkdir(finfo.completeBaseName());
  outDir.cd(finfo.completeBaseName());
  QString program = "gs";
  QStringList arguments;
  arguments << "-Ilib" << "-Ifonts"
	    << "-dBATCH" 
	    << "-dQUIET" 
	    << "-sDEVICE=png16m" 
	    << "-r"+QString::number(dpi) 
	    << QString("-sOutputFile=")+ outDir.canonicalPath()+ QString("/%05d.png")
	    << finfo.canonicalFilePath();// <<  "&";

  //  std::cout << arguments.join(" ").toStdString() << std::endl;
  //  std::cout << "gs " <<	"-dBATCH " << "-dQUIET" << "-sDEVICE=png16m " << "-r"+QString::number(dpi).toStdString() 
  //	    << "-sOutputFile=\""+outDir.canonicalPath().toStdString()+"/%05d.png\""
  //	    << finfo.canonicalFilePath().toStdString();
	
  QProcess *myProcess = new QProcess;
  myProcess->setReadChannel(QProcess::StandardOutput);
  myProcess->start(program, arguments);

  QProgressDialog progress("Converting document "+QString::number(filenum)+" of "+QString::number(filecount)+" from ps to png", "Abort", 0, 100, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  int i=0;    
  for (;;) {
    qApp->processEvents();

    if (progress.wasCanceled()) {
      myProcess->kill();
      delete myProcess;
      return -239;
    }
    char buf[1024];
    qint64 lineLength = myProcess->readLine(buf, sizeof(buf));
    if (lineLength>0) {

      if (strncmp(">>showpage",buf,10)==0){
	myProcess->write("\n\r",strlen("\r\n"));
	i++;
	if (i==100) i=50;
	progress.setValue(i);
      }
      //      std::cout << buf << std::endl;
    }	
    if (myProcess->waitForFinished(50)) {
      //      std::cout << "finished " << std::endl;
      break;
    }

  }
  progress.setValue(100);
  int exc=myProcess->exitCode();
  QByteArray result = myProcess->readAllStandardOutput();
  //  std::cout << QString(result).toStdString() << std::endl;
  result = myProcess->readAllStandardError();
  //	std::cout << QString(result).toStdString() << std::endl;
  //  std::cout << " Exit code:" << exc << std::endl;
  delete myProcess;
  return exc;
}

int MainWindow::djvuToPs(QFileInfo finfo, int filenum, int filecount) {
  QDir outDir(outputDir);
  QString program = "djvups";
  QStringList arguments;
  arguments << "-verbose" << finfo.canonicalFilePath() << outDir.canonicalPath()+"/"+finfo.completeBaseName()+".ps";
  //	std::cout << "djvups ";
  //	std::cout << QString("\""+ finfo.canonicalFilePath()+"\"").toStdString() << QString("\""+outDir.canonicalPath()+"/"+finfo.completeBaseName()+".ps\"").toStdString() << std::endl;

  QProcess *myProcess = new QProcess;
  myProcess->setReadChannel(QProcess::StandardError);
  myProcess->start(program, arguments);

  QProgressDialog progress("Converting document "+QString::number(filenum)+" of "+QString::number(filecount)+" from djvu to ps", "Abort", 0, 50, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  for (;;) {
    qApp->processEvents();
    if (progress.wasCanceled()) {
      myProcess->kill();
      delete myProcess;
      return -239;
    }
    //	std::cout << i << " 1\n";
    //	QByteArray result = myProcess->readAllStandardOutput();
    //	std::cout << QString(result).toStdString() << std::endl;
    char c;
    int j=0;
    while (myProcess->getChar(&c))  {
      if (c=='#') {
	j++;
      }
      //			std::cout << c;
    }
    if (j!=0) progress.setValue(j);

    if (myProcess->waitForFinished(50)) {
      //				std::cout << "finished " << std::endl;
      break;
    }

  }
  progress.setValue(50);
  int exc=myProcess->exitCode();
  //	std::cout << " Exit code:" << exc << std::endl;
  delete myProcess;
  return exc;

}

int MainWindow::pngToPng(QDir inpDir, QDir outDir,int filenum, int filecount) {
  QString html="<html><head><title>"+outDir.dirName()+"_part_1"+"</title></head><body>\n";
  QFileInfoList imgList=inpDir.entryInfoList(QDir::Files);
  QList<QFileInfo>::iterator it;
  int size=imgList.size();
  int num=0;
  int i=0;

  //  std::cout << "Inp dir:" << inpDir.canonicalPath().toStdString() << " Out dir:" << outDir.canonicalPath().toStdString() << std::endl;

  QProgressDialog progress("Resizing document "+QString::number(filenum)+" of "+QString::number(filecount)+" to fit into PDA screen", "Abort", 0, size, this);

  for (it=imgList.begin();it!=imgList.end();++it) {
    qApp->processEvents();
    if (progress.wasCanceled()) return -239;
		
    i++;
    progress.setValue(i);

    QList<QImage> *images=imageProcessor->processImage((*it).canonicalFilePath());
    //    std::cout << (*it).canonicalFilePath().toStdString() << std::endl;

    QString fname=(*it).completeBaseName();
    QList<QImage>::iterator imgIt;
    int ind=0;
    for (imgIt=images->begin();imgIt!=images->end();++imgIt) {
      QString imageName=fname+"_"+QString::number(ind)+".png";
      (*imgIt).save(outDir.canonicalPath()+"/"+imageName,"png");
      ind++;
      num++;
      //      std::cout << "Num:" << num << " chunk size:" << chunkSize << std::endl;
      html+="<img src=\""+imageName+"\" /><br>\n";
      if (num % chunkSize==0) {
	html+="</body></html>";
	QFile htmlFile(outDir.canonicalPath()+"/"+outDir.dirName()+"_part_"+QString::number(num/chunkSize)+".html");
	htmlFile.open(QIODevice::WriteOnly);
	QTextStream os(&htmlFile);
	os << html;
	htmlFile.close();
	html="<html><head><title>"+outDir.dirName()+QString::number(num/chunkSize+1)+"</title></head><body>\n";
      }
    }
    delete images;
  }
  html+="</body></html>";
  QFile htmlFile(outDir.canonicalPath()+"/"+outDir.dirName()+"_part_"+QString::number(num/chunkSize+1)+".html");
  htmlFile.open(QIODevice::WriteOnly);
  QTextStream os(&htmlFile);
  os << html;
  htmlFile.close();
  return 0;
}	

void MainWindow::convert() {
  QList<QString>::iterator i;
  int size=fileNamesList->size();
  int num=0;
  for (i = fileNamesList->begin(); i != fileNamesList->end(); ++i){
    num++;	
    QString str = (*i);
    QFileInfo finfo(str);
    //    std::cout << str.toStdString() << std::endl;
    if ((finfo.suffix().toLower()=="pdf") || (finfo.suffix().toLower()=="ps")) {
      int res= psToPng(finfo,num,size);
      if (res==-239) return;
      if (res!=0) continue;	
      QDir outDir(outputDir);
      outDir.cd(finfo.completeBaseName());
      if (pngToPng(outDir,outDir,num,size)==-239) return;
    } else if (finfo.suffix().toLower()=="djvu") {
      QDir outDir(outputDir);
      int res = djvuToPs(finfo,num,size);
      if (res==-239) return;
      if (res!=0) continue;
      res= psToPng(QFileInfo(outDir.canonicalPath()+"/"+finfo.completeBaseName()+".ps"),num,size);
      if (res==-239) return;
      if (res!=0) continue;	
      outDir.cd(finfo.completeBaseName());
      if (pngToPng(outDir,outDir,num,size)==-239) return;

    } else if (finfo.isDir()) {
      QDir outDir(outputDir);
      outDir.mkdir(finfo.completeBaseName());
      outDir.cd(finfo.completeBaseName());

      pngToPng(QDir(finfo.canonicalFilePath()),outDir,num,size);
    }

  }
}

void MainWindow::createActions()
{
  addAct = new QAction(QIcon(":/images/add.png"), tr("&Add"), this);
  addAct->setText(tr("Add"));
  addAct->setShortcut(tr("Ctrl+A"));
  addAct->setStatusTip(tr("Add book to convert"));
  connect(addAct, SIGNAL(triggered()), this, SLOT(addFile()));

  addDirAct = new QAction(QIcon(":/images/adddir.png"), tr("Add &dir"), this);
  addDirAct->setText(tr("Add"));
  addDirAct->setShortcut(tr("Ctrl+A"));
  addDirAct->setStatusTip(tr("Add book to convert"));
  connect(addDirAct, SIGNAL(triggered()), this, SLOT(addDir()));
	
  clearAct = new QAction(QIcon(":/images/clear.png"), tr("C&lear"), this);
  clearAct->setShortcut(tr("Ctrl+X"));
  clearAct->setStatusTip(tr("Clear books list"));
  connect(clearAct, SIGNAL(triggered()), this, SLOT(clear()));

  convertAct = new QAction(QIcon(":/images/convert.png"), tr("&Convert"), this);
  convertAct->setShortcut(tr("Ctrl+C"));
  convertAct->setStatusTip(tr("Convert books"));
  connect(convertAct, SIGNAL(triggered()), this, SLOT(convert()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(addAct);
  fileMenu->addAction(addDirAct);
  fileMenu->addAction(clearAct);
  fileMenu->addSeparator();
  fileMenu->addAction(convertAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(addAct);
  fileToolBar->addAction(addDirAct);
  fileToolBar->addAction(clearAct);
  fileToolBar->addAction(convertAct);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

void MainWindow::changeWidth(int newWidth) {
  imageProcessor->width=newWidth;
}
void MainWindow::changeDpi(int newDpi) {
  dpi=newDpi;
}
void MainWindow::changeChunkSize(int newChunkSize){
  chunkSize=newChunkSize;
}

void MainWindow::selectOutputDir() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
						  outputDir,
						  QFileDialog::ShowDirsOnly
						  | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty()) {
    outputDir=dir;
    outEdit->setText(dir);
  }
}

MainWindow::~MainWindow() {
  delete imageProcessor;
  delete fileNamesList;
  delete fileList;
}

