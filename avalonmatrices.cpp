#include "avalonmatrices.h"

#include <typeinfo>
#include <QString>

void mTest(){
    qDebug()<<"mTest() started.";

    arma::mat A = arma::randu<arma::mat>(4,5);
    arma::mat B = arma::randu<arma::mat>(5,4);
    arma::mat C = A*B;
    printInfoToDebug(C);

    C = C.i();

    printInfoToDebug(C);
    qDebug()<<"mTest() completed.";
}

//Matrices are going to be y, x.  Meaning that it will be rows x columns.
void printInfoToDebug(arma::mat m){
    QString row;
    for (size_t i = 0; i < m.n_rows; i++){
        row.clear();
        for (size_t j = 0; j < m.n_cols; j++){
            //Returns a double.
            row += QString::fromStdString(std::to_string(m(i,j)));
            if (j == m.n_cols-1)
                qDebug()<<row;
            else
                row += ", ";
        }
    }
}

//Cycle through nodes and assign the codeNumbers.  Then cycle through elements[i].nodes and apply the nodes created previously.
/*
******** Make the code numbers by this:
******** Add codeX and codeY members to node struct.  Initalise to -1.
******** Initalise an int ticker to 1.
******** Cycle through nodes global vector.
********    for x, y, z part of each node
********        If fixed (check dof)
********            that code = 0
********        Else
********            code = ticker, ticker++
*/
void assignCodeNumbers(std::vector<element> &elements, std::vector<node> &nodes){
    int ticker = 1;
    //Constrained translation in x.
    bool constrainedTransX = false;
    bool constrainedTransY = false;
    bool constrainedTransZ = false;
    for (size_t i = 0; i < nodes.size(); i++){
        //Avoid wasting space with an else statement at bottom.
        nodes[i].codeX = 0;
        nodes[i].codeY = 0;
        nodes[i].codeZ = 0;

        //if (nodes[i].degreesOfFr)
        for (size_t j = 0; j < nodes[i].degreesOfFreedom.size(); j++){
            /*degreeOfFreedom
            * int identifier;
            * int value;
            * 1,2,3 are x,y,z translation.  Then 4-6 are x,y,z rotation.
            * value = 0 if constrained.
            */
            if (nodes[i].degreesOfFreedom[j].identifier == 1 && nodes[i].degreesOfFreedom[j].value == 0)
                constrainedTransX = true;
            if (nodes[i].degreesOfFreedom[j].identifier == 2 && nodes[i].degreesOfFreedom[j].value == 0)
                constrainedTransY = true;
            if (nodes[i].degreesOfFreedom[j].identifier == 3 && nodes[i].degreesOfFreedom[j].value == 0)
                constrainedTransZ = true;
        }
        //We have finished looping through the degreesOfFreedom and now know in what directions node is constrained.
        if (!constrainedTransX){
            nodes[i].codeX = ticker;
            ticker++;
        }
        if (!constrainedTransY){
            nodes[i].codeY = ticker;
            ticker++;
        }
        if (!constrainedTransZ){
            nodes[i].codeZ = ticker;
            ticker++;
        }
    }

    //We have now looped through nodes vector.  We now proceed to loop through the elements vector (note passed by reference).
    for (size_t i = 0; i < elements.size(); i++){
        for (size_t j = 0; j < elements[i].subelements.size(); j++){
            for (size_t k = 0; k < elements[i].subelements[j].nodes.size(); k++){
                //Once we are looping through the node level of each element, we loop through the nodes vector that we applied codes to in the above part of this function.
                for (size_t x = 0; x < nodes.size(); x++){
                    //We check if the node of elements and node of node vector share the same identifier.  If they do, we copy value with codes.
                    if (nodes[x].name == elements[i].subelements[j].nodes[k].name){
                        //Note: check if I need to not use the default = operator.
                        elements[i].subelements[j].nodes[k] = nodes[x];
                        //One subelement will not have multiple instances of the same node in its nodes vector member, so if we've hit it we can break out of the loop.
                        break;
                    }
                }
            }
        }
    }


}
