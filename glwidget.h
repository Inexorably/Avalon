#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QObject>

class GLWidget : public QGLWidget{
    Q_OBJECT
public:
    GLWidget();
    explicit GLWidget(QWidget *parent = 0);

    void initializeGL();
    void paintGL();
    void resizeGL(int h, int w);
};

#endif // GLWIDGET_H
