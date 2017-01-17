#include "elements.h"

#include <QDebug>

//Constructor for node struct.
node::node(long double a, long double b, long double c){
    x = a;
    y = b;
    z = c;
    node();
}

//Default constructor.
node::node(){
    codeX = -1;
    codeY = -1;
    codeZ = -1;
}

//Initalises to no direction.
directionVector::directionVector(){
    x = 0;
    y = 0;
    z = 0;
}

//Prints the debug info to qDebug().
void node::printDebugInfo(){
    qDebug()<<"Name: "<<QString::fromStdString(name);
    qDebug()<<"Name Size: "<<name.size();
    qDebug()<<"x: "<<(double)x;
    qDebug()<<"y: "<<(double)y;
    qDebug()<<"z: "<<(double)z;
}
