#include "utilities.h"

#include <string>
#include <algorithm>
#include <sstream>
#include <QDebug>

#include "globals.h"

#include <Eigen>
#include <armadillo>

//Using the eigen functions we solve an arma::mat.
//Ax = B
arma::mat eigenArmaSolve(arma::mat A, arma::mat B){
    Eigen::MatrixXd eigenA(A.n_rows, A.n_cols);
    for (size_t i = 0; i < A.n_rows; i++){
        for (size_t j = 0; j < A.n_cols; j++){
            eigenA(i,j) = A(i,j);
        }
    }
    //std::cout << 'a' << std::endl;
    //std::cout << eigenA << std::endl;
    Eigen::MatrixXd eigenB(B.n_rows, B.n_cols);
    for (size_t i = 0; i < B.n_rows; i++){
        for (size_t j = 0; j < B.n_cols; j++){
            eigenB(i,j) = B(i,j);
        }
    }
    //std::cout << 'b' << std::endl;
    //std::cout << eigenB << std::endl;
    //We have now converted the arma matrices to eigen. LU as square.
    Eigen::MatrixXd x = eigenA.colPivHouseholderQr().solve(eigenB);
    arma::mat temp = arma::zeros(B.n_rows, B.n_cols);
    for (size_t i = 0; i < B.n_rows; i++){
        for (size_t j = 0; j < B.n_cols; j++){
            temp(i,j) = x(i,j);
        }
    }
    //std::cout << 'x' << std::endl;
    //std::cout << x << std::endl;
    return temp;
}


//Checks if string a begins with string b.
bool beginsWith(std::string a, std::string b){
    for (size_t t = 0; t < b.size(); t++){
        if (a[t] != b[t])
            return false;
    }
    return true;
}

//Converts the string line to uppercase.  To be used for comparisons to things like NODE to prevent a mistype such as *Node from preventing compile.
std::string uppercase(std::string line){
    std::string temp = line;
    std::transform(temp.begin(), temp.end(),temp.begin(), ::toupper);
    return temp;
}

//Remove all the whitespaces from a string, passes by reference so no return needed.
void removeWhiteSpaces(std::string &line){
    removeCharacter(line, ' ');
}

//Remove all instances of char c from string.
void removeCharacter(std::string &line, char c){
    for (size_t t = 0; t < line.size(); t++)
        if (line[t] == c)
            line.erase(line.begin()+t);
}

//Remove all commas from a string.
void removeCommas(std::string &line){
    removeCharacter(line, ',');
}

//The current Element name is a string used in the load function to know what element we are setting attributes for.
//This is used due to the design of Abaqus using the ELSET= function.
//Returns the index as an int, for use in the elements vector in the load function.
//Note: said vector could get externed at some point.
//Returns -1 if not found.
int getCurrentElementIndex(std::vector<element> elements, std::string currentElementName){
    for (size_t t = 0; t < elements.size(); t++){
        if (elements[t].name == currentElementName)
            return t;
    }
    return -1;
}

//Same thing for materials. Oh it's the same thing lmfao.  Well this is more readable than templating it's okay I guess.
int getCurrentMaterialIndex(std::vector<material> materials, std::string currentMaterialName){
    for (size_t t = 0; t < materials.size(); t++){
        if (materials[t].name == currentMaterialName)
            return t;
    }
    return -1;
}

//The modulus can be input as a string (10e6) in abaqus, so we need a function that converts from scientific to double.
//Splits by char e.
long double scientificToDouble(std::string s){
    std::stringstream stream(s);
    std::string temp;
    std::vector<std::string> split;


    while(std::getline(stream, temp, 'e')){
       //Shouldn't be a problem but just in case:
       removeCharacter(temp, 'e');
       split.push_back(temp);
    }

    long double a;
    long double b;

    a = std::stod(split[0]);
    b = std::stod(split[1]);

    return a * pow(10, b);
}

//Convert from the axis format direction (example: 1 is x, 2 is y....) of Abaqus to radians.
//In unit vector format.
directionVector axisToDirectionVector(double input){
    //Need a 3d vector describing the direction.  Will make it take more inputs once I figure out how to exactly the abaqus angle works.
    directionVector temp;
    if (input == 1){
        temp.x = 1;
    }
    else if (input == 2){
        temp.y = 1;
    }
    else if (input == 3){
        temp.z = 1;
    }
    else
        qDebug()<< "Error in axisToDirectionVector function.\n";
    return temp;
}

//Wipes the globals for when we need to load a new file.
void wipeGlobals(){
    elements.clear();
    steps.clear();
    materials.clear();
    nodes.clear();
}
