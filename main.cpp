#include "mainwindow.h"
#include "loadingwindow.h"
#include <QApplication>
#include <QFile>
#include <QSplashScreen>
#include <QTime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    LoadingWindow loadingWindow(QPixmap(":/mando.jpg"));
    loadingWindow.show();


    QTime time;
    time.start();
    for (int i = 0; i < 100; )
    {
        if (time.elapsed() > 40)
        {
            time.start();
            ++i;
            loadingWindow.setProgress(i);
        }
        loadingWindow.setMessage("Loading modules: " + QString::number(i) + "%");
        qApp->processEvents();
    }


    loadingWindow.finish(nullptr);

    MainWindow w;
    w.show();

    return a.exec();
}

