#include <iostream>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>

#include <QFileDialog>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QUrl>
#include <QTime>
#include <QtGui/QApplication>
#include <QtGui/QDirModel>
#include <QtGui/QTreeView>
#include <QtGui/QFileSystemModel>
#include <QMainWindow>
#include <QtGui>

QT_BEGIN_NAMESPACE
class QAction;
class QTableWidget;
class QLCDNumber;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow(char *file);

    private slots:
        void tick(void);
        void totalTimeUpdate(qint64 time);
        void file_selected(const QItemSelection& selection);

    private:
        void mytick(qint64 time);
        Phonon::MediaObject* media;
        QTreeView *tree;
        QLCDNumber *timeLcd;
        Phonon::SeekSlider *seekSlider;
};

MainWindow::MainWindow(char *file)
{
        
    QFileSystemModel *model = new QFileSystemModel;
    tree = new QTreeView;

    model->setRootPath(QDir::currentPath());

    tree->setModel(model);

    tree->setRootIndex(model->index(file));
    tree->setColumnHidden( 1, true );
    tree->setColumnHidden( 2, true );
    tree->setColumnHidden( 3, true );

    //tree->setWindowTitle(QObject::tr("Dir View:")+QDir::homePath());
    //tree->resize(640, 480);
    tree->show();

    media = new Phonon::MediaObject(this);

    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setMediaObject(media);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addWidget(seekSlider);
    mainLayout->addWidget(timeLcd);

    QWidget *widget = new QWidget;
    widget->setLayout(mainLayout);

    setCentralWidget(widget);
    setWindowTitle("Phonon Music Player");

    connect(tree->selectionModel(), SIGNAL(selectionChanged(const
                                    QItemSelection&, const QItemSelection &)),
                    this, SLOT(file_selected(const QItemSelection &)));

    Phonon::createPath(media, new Phonon::AudioOutput(Phonon::MusicCategory, this));
    //media->setCurrentSource(QFileDialog::getOpenFileName(0,QString("Select a file to play"),QString()));
    media->setTickInterval(1000);
    connect(media, SIGNAL(tick(qint64)), this, SLOT(tick(void)));
    connect(media, SIGNAL(totalTimeChanged(qint64)), this,
                    SLOT(totalTimeUpdate(qint64)));


}

void MainWindow::mytick(qint64 time) {
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    std::cout << displayTime.toString("mm:ss").toStdString() << std::endl;
    timeLcd->display(displayTime.toString("mm:ss"));

}

void MainWindow::tick(void) {
    mytick(media->remainingTime());
}

void MainWindow::totalTimeUpdate(qint64 time) {
        mytick(time);
}

void MainWindow::file_selected(const QItemSelection& selection) {
        media->stop();
        QModelIndexList list = selection.indexes();
        QFileSystemModel *model = (QFileSystemModel*) tree->model();
        int row = -1;
        foreach (QModelIndex index, list) {
                if (index.row()!=row && index.column()==0)
                {
                        QString filename = model->filePath(index);
                        qDebug() << filename;
                        row = index.row();
                        media->setCurrentSource(filename);
                        media->play();
                }
        }
}

int main(int argc, char **argv)
{
        if (argc < 2) {
                printf ("Usage: %s /path/to/mp3\n", argv[0]);
                exit (1);
        }
        QApplication app(argc, argv);
        QApplication::setApplicationName("Phonon Tutorial 2");
        MainWindow mw(argv[1]);
        mw.show();
        return app.exec();
}

#include "main.moc"
