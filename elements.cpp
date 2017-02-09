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
    for (int i = 0; i < 3; i++)
        displacements[i] = 0;
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

//Returns the length between the two nodes.
long double subelement::length(){
    return sqrt(pow((nodes[1].x-nodes[0].x),2)+pow((nodes[1].y-nodes[0].y),2)+pow((nodes[1].z-nodes[0].z),2));
}

//The direction cosines used for extension to 3d.
//Making it a return function not a storage for portability and as increased processing time is small (two operations, length, - + /).
long double subelement::cx(){
    return (nodes[1].x-nodes[0].x)/length();
}

long double subelement::cy(){
    return (nodes[1].y-nodes[0].y)/length();
}

long double subelement::cz(){
    return (nodes[1].z-nodes[0].z)/length();
}
