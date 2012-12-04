
#include "MainWindow.h"
#include "ui_mainwindow.h"

//#include "myglframe.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //GLFrame = new MyGLFrame();      // create our subclassed GLWidget
    //setCentralWidget(GLFrame);      // assign it to the central widget of the window
    //GLFrame->initRenderThread();    // start rendering
}

MainWindow::~MainWindow()
{
    //delete GLFrame;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    //GLFrame->stopRenderThread();    // stop the thread befor exiting
    QMainWindow::closeEvent(evt);
}
