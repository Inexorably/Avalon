#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //This sets the errorTextBrowser text to display string s for when the input file is formatted incorrectly.
    void errorInLoadFileButton(std::string s);

private slots:
    void on_loadFileButton_clicked();
    void on_displayElementsButton_clicked();

    void on_processButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
