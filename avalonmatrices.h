#ifndef AVALONMATRICES_H
#define AVALONMATRICES_H

#include <armadillo>
#include <QDebug>
#include <vector>
#include <typeinfo>
#include <QString>

#include "utilities.h"
#include "elements.h"

void mTest();

void printInfoToDebug(arma::mat m);

//Cycle through nodes and assign the codeNumbers.  Then cycle through elements[i].nodes and apply the nodes created previously.
//Assigns the code numbers and returns the forceVector with order corresponding to the code numbers.
arma::mat assignCodeNumbersAndCreateForceVector(std::vector<element> &elements, std::vector<node> &nodes);

//Cycle through elements and their subelements and create member stiffness matrix K.  Note that we need to have access to both the
//elements vector and the subelements of those elements (can't just make member function of subelement) because we need access to A and E
//which are applied on an element level.
void createMemberStiffnessMatrices(std::vector<element> &elements);

//Creates a global stiffness matrix K.
arma::mat createGlobalStiffnessMatrix(std::vector<element> &elements, const std::vector<node> nodes, int stiffnessSize);

//KD=Q
//D=K'Q

//Note: 2D only for use of beta.  3d uses cosine angles.
//Todo :
    //function that returns Q from nodes vector
        //(cycles through all the vectors and returns the force magnitude on each.
        //Account for non existant / empty force vector (means no force).

    //D = K.i()*Q;
        //Don't even need to specify dimenions on initalization, done by = arma::mat operator.

    //Forces on each member after displacement calculated.
        //https://ccle.ucla.edu/pluginfile.php/1657598/mod_resource/content/0/Trusses.pdf, page 30.  All the variables for this are known at this point.

//ToDo End

//3D Notes
/*
        Explicitly, K for each member is:
        https://ccle.ucla.edu/pluginfile.php/1657598/mod_resource/content/0/Trusses.pdf, page 52

        Note that this is the member stiffness matrix.  We then have 2 nodes with 3 numbers each for codes so 6 codes.

        We then need to figure out the codes for each members.

        We then create matrix D as empty:
        Global Displacement matrix.
        D1 D2..... D6  Displacement of eachnode. nx1.

        NOTE: n = number of nodes.  0 if fixed.

        K member will always be 6x6.  Then using the code numbers, we convert all the member matrices into K global.

********Make the code numbers by this:
******** Add codeX and codeY members to node struct.  Initalise to -1.
******** Initalise an int ticker to 1.
******** Cycle through nodes global vector.
********    for x, y, z part of each node
********        If fixed (check dof)
********            that code = 0
********        Else
********            code = ticker, ticker++


        We then create K global which will be nxn using the above code numbers.

        Q is cload matrix.  Magnitude of cload on each of nodes.  nx1.

        D starts as empty, nx1.

        D = K^-1*Q.

*/

/*Unimportant stuff
 * //==================================================For 2D===================================================//

//member stiffness matrix structure
    //matrix (nxn)
    //member name
    //code number (n codes)
    kc2   ksc     -kc2    -ksc
     *sym   ks2     -ksc    -ks2
     *sym   sym     kc2     ksc
     *sym   sym     sym     ks2



//Constructor from subelement
    //need k = EA/L and B (the angle).

//Need a function to figure out and set B (returns double)
//Need a function to figure out the code number (returns std::vector<int> codes, size() == 4)
//Need a function that takes B as input and returns 4x4 arma::mat.

//global stiffness matrix structure
    //std::vector<member stiffness matrix structures> memberStiffnessMatrices;

    //Once all member stiffness matrices have been created, we need a function to set the global stiffness matrix.
    //arma::mat stiffnessMatrix, size == nxn (number of the highest code in its memberStiffnessMatrices members).


//Load matrix
    //number of nodes x 1 matrix, has the load magnitudes.

//Displacement matrix
    //Once global stiffness and load are finished creating:
    //D = K^-1Q




//Unimportant stuff
*/
#endif // AVALONMATRICES_H
