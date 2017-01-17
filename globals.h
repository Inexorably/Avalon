#ifndef GLOBALS_H
#define GLOBALS_H

#include "elements.h"

#include <vector>
#include <string>

extern bool globalDebugLoadButtonSectionTags;

//This will store the elements read from the following fstream loop.
//Note: 1/14 moved from load function to global status.
extern std::vector<element> elements;

//This string will include all step and all the following parameters.
//Note: Contains the keyword static (or what keyword(s) are directly below the line that *Step is used.
extern std::vector<std::string> stepVector;

//A materials vector as the material modulus and poisson are defined seperately from element material definition.
//After a material is added to the materials vector, we will cycle through the elements vector and update all mat members of type material
//for the newly added material.
extern std::vector<material> materials;

//Step vector.
extern std::vector<step> steps;

//Vector to store our nodes into, which will then be read into the elements vector.  To save ram, this vector can be cleared after transfer to elements.
extern std::vector<node> nodes;

//Tells if we have run the processing function yet or not (relevant to display node code numbers).
extern bool processingComplete;

#endif // GLOBALS_H
