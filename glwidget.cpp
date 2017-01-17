#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent){

}

void GLWidget::initializeGL(){
    glClearColor(1, 1, 0, 1);
}

void GLWidget::paintGL(){

}

void GLWidget::resizeGL(int h, int w){

}
