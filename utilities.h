#ifndef UTILITIES_H
#define UTILITIES_H

#include "elements.h"

#include <string>
#include <vector>

//Solves Ax=B.
arma::mat eigenArmaSolve(arma::mat A, arma::mat B);

//To prevent dumb errors from misspelling strings hardcoded, use variable names for logic.
const std::string NOERROR = "NOERROR";
const std::string NODE = "*NODE";
const std::string ELEMENT = "*ELEMENT";
const std::string SOLID_SECTION = "*SOLID SECTION";
const std::string MATERIAL = "*MATERIAL";
const std::string ELASTIC = "*ELASTIC";
const std::string STEP = "*STEP";
//Note: made part of the step section.
const std::string STATIC = "*STATIC";
const std::string BOUNDARY = "*BOUNDARY";
const std::string CLOAD = "*CLOAD";
const std::string END = "*END";


//Checks if string a begins with string b.
bool beginsWith(std::string a, std::string b);

//Converts the string line to uppercase.  To be used for comparisons to things like NODE to prevent a mistype such as *Node from preventing compile.
std::string uppercase(std::string line);

//Remove all the whitespaces from a string, passes by reference so no return needed.
void removeWhiteSpaces(std::string &line);

//Removes all instances of char c from string.
void removeCharacter(std::string &line, char c);

//Remove all instances of comma from string.
void removeCommas(std::string &line);

//The current Element name is a string used in the load function to know what element we are setting attributes for.
//This is used due to the design of Abaqus using the ELSET= function.
//Returns the index as an int, for use in the elements vector in the load function.
//Note: said vector could get externed at some point.
//Returns -1 if not found.
int getCurrentElementIndex(std::vector<element> elements, std::string currentElementName);

//Same thing for elements.
int getCurrentMaterialIndex(std::vector<material> materials, std::string currentMaterialName);

//The modulus can be input as a string (10e6) in abaqus, so we need a function that converts from scientific to double.
long double scientificToDouble(std::string s);

//Convert from the axis format direction (example: 1 is x, 2 is y....) of Abaqus to direction vector (unit).
directionVector axisToDirectionVector(double input);

//Wipes the globals for when we need to load a new file.
void wipeGlobals();

#endif // UTILITIES_H
