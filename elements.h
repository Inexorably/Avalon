#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <string>
#include <vector>
#include <armadillo>

//Initalises to no direction.
struct directionVector{
    long double x;
    long double y;
    long double z;
    directionVector();
};

struct step{
    std::vector<std::string> parameters;
};

struct material{
    std::string name;
    long double youngsModulus;
    long double poissonRatio;
    //Vector containing properties such as elastic / dynamic / etc.
    std::vector<std::string> properties;
};

struct degreeOfFreedom{
    //1-6.  3d translation and 3d rotation.
    //translation x y z rotation x y z.
    int identifier;
    int value;
};

struct force{
    directionVector direction;
    long double magnitude;
};

struct node{
    //The node identifier.
    std::string name;
    //The code numbers for each of the respective directions in the node.
    int codeX, codeY, codeZ;
    //Coordinates of the node.
    long double x, y, z;
    //Constructor.
    node(long double x, long double y, long double z);
    //Default constructor
    node();
    //Positive x y and z displacements.
    long double displacements[3];
    //DOF constraints.  Needs to be initalised.
    std::vector<degreeOfFreedom>degreesOfFreedom;

    //A vector for the forces on the node.
    std::vector<force> forces;

    //Prints the debug info to qDebug().
    void printDebugInfo();
};

//A substruct of element, as one element can have different subelements related to nodes.
struct subelement{
    std::string name;
    std::vector<node> nodes;
    //Returns the length between the two nodes.
    long double length();

    //The basic forces vector.
    arma::mat basicForces;

    arma::mat displacementMatrix;

    //The direction cosines used for extension to 3d.
    //Making it a return function not a storage for portability and as increased processing time is small (two operations, - and /).
    long double cx();
    long double cy();
    long double cz();

    arma::mat T;

    //The member matrix K is a member of subelement.  Relies on armadillo library.
    arma::mat K;

    //Repeatedly computing

    /*
     In future may have to make extendable subelement class.  Reminder / note:
     std::vector<subelement> subelements;
     */
};

struct element{
    //Abaqus allows an element to have subelements such as seen in the following clip:
    /*
    *ELEMENT, TYPE=T2D2, ELSET=FRAME
    * **Subelement name, node number, node number....
    11, 101, 102
    12, 102, 103
    */
    std::vector<subelement> subelements;
    std::string type;
    std::string name;
    std::string materialString;
    std::string crossSectionType;
    long double area;
    //Because the material is defined after the material name of an element is given, we will store the material name first, then as a material is defined
    //We wills search the elements vector and assign its values to the proper elements[i].mat members.
    //Applied on element not subelement level.
    material mat;


    /*Removed Features, keeping for reference
    //We can have any number of nodes attached to an element.
    //std::vector<node> nodes;
    //Note: Moved to subelement.
    */
};

#endif // ELEMENTS_H
