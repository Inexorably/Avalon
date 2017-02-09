#include "avalonmatrices.h"

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
//Returns the largest code number, for use in determing size of global stiffness matrix K.
arma::mat assignCodeNumbersAndCreateForceVector(std::vector<element> &elements, std::vector<node> &nodes){
    //We want to return the individual directions with the magnitudes in order.
    //Can still return size.
    //We can just return the forceVector as arma::mat. Easy.
    std::vector<long double> forceVector;

    int ticker = 1;
    for (size_t i = 0; i < nodes.size(); i++){
        //Avoid wasting space with an else statement at bottom.
        nodes[i].codeX = 0;
        nodes[i].codeY = 0;
        nodes[i].codeZ = 0;

        //Constrained translation in x.
        bool constrainedTransX = false;
        bool constrainedTransY = false;
        bool constrainedTransZ = false;

        //if (nodes[i].degreesOfFr)
        //qDebug()<<"Node_: " << QString::fromStdString(nodes[i].name);
        //qDebug()<<"dof size: "<<QString::fromStdString(std::to_string(nodes[i].degreesOfFreedom.size()));
        for (size_t j = 0; j < nodes[i].degreesOfFreedom.size(); j++){
            /*degreeOfFreedom
            * int identifier;
            * int value;
            * 1,2,3 are x,y,z translation.  Then 4-6 are x,y,z rotation.
            * value = 0 if constrained.
            */
            //qDebug()<<"Node__: " << QString::fromStdString(nodes[i].name);
            //QString tempDebugQstring = QString::fromStdString(std::to_string(nodes[i].degreesOfFreedom[j].identifier)) + ": " + QString::fromStdString(std::to_string(nodes[i].degreesOfFreedom[j].value));
            //qDebug() << tempDebugQstring;
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
            //We combine all the forces in one direction and apply them to this direction in the forceVector.
            long double tempDouble = 0;
            for (size_t j = 0; j < nodes[i].forces.size(); j++){
                //The force magnitude times the unit vector to correctly scale the force.
                tempDouble += (nodes[i].forces[j].magnitude*nodes[i].forces[j].direction.x);
            }
            //We have summed all of the forces in one direction in the above loop.
            forceVector.push_back(tempDouble);
            ticker++;
        }
        if (!constrainedTransY){
            nodes[i].codeY = ticker;
            //We combine all the forces in one direction and apply them to this direction in the forceVector.
            long double tempDouble = 0;
            for (size_t j = 0; j < nodes[i].forces.size(); j++){
                //The force magnitude times the unit vector to correctly scale the force.
                tempDouble += (nodes[i].forces[j].magnitude*nodes[i].forces[j].direction.y);
            }
            //We have summed all of the forces in one direction in the above loop.
            forceVector.push_back(tempDouble);
            ticker++;
        }
        if (!constrainedTransZ){
            nodes[i].codeZ = ticker;
            //We combine all the forces in one direction and apply them to this direction in the forceVector.
            long double tempDouble = 0;
            for (size_t j = 0; j < nodes[i].forces.size(); j++){
                //The force magnitude times the unit vector to correctly scale the force.
                tempDouble += (nodes[i].forces[j].magnitude*nodes[i].forces[j].direction.z);
            }
            //We have summed all of the forces in one direction in the above loop.
            forceVector.push_back(tempDouble);
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
    //qDebug()<<"Force vector size: " << (int)forceVector.size();
    arma::mat returnMat = arma::zeros(1,(int)forceVector.size());
    for (size_t i = 0; i < forceVector.size(); i++){
        //Divide by 1000 because of units.
        returnMat(0, i) = forceVector[i]/1000;
    }
    //Return the force vector.
    //qDebug()<< "assignation complete.";
    return returnMat;
}

//Cycle through elements and their subelements and create member stiffness matrix K.  Note that we need to have access to both the
//elements vector and the subelements of those elements (can't just make member function of subelement) because we need access to A and E
//which are applied on an element level.
void createMemberStiffnessMatrices(std::vector<element> &elements){
    for (size_t i = 0; i < elements.size(); i++){
        for (size_t j = 0; j < elements[i].subelements.size(); j++){
            //We are now cycling through the subelements (j) of element i.
            //Each subelement j has a member K of type arma::mat.
            arma::mat tempMat;
            long double cx = elements[i].subelements[j].cx();
            long double cy = elements[i].subelements[j].cy();
            long double cz = elements[i].subelements[j].cz();
            //Taking break just fill K as seen on slide page 51 of trusses and times EA/L.
            arma::mat T;
            T.zeros(2,6);
            T(0,0) = cx;
            T(0,1) = cy;
            T(0,2) = cz;
            T(1,3) = cx;
            T(1,4) = cy;
            T(1,5) = cz;
            elements[i].subelements[j].T=T;
            arma::mat kp;
            kp.ones(2,2);
            kp(0,1) = -1;
            kp(1,0) = -1;
            tempMat = T.t()*kp*T;
            //qDebug() << QString::fromStdString(elements[i].subelements[j].name);
            elements[i].subelements[j].K = (elements[i].area*elements[i].mat.youngsModulus/*Divide by 10e3 for units (slide 26 truss pdf*//1000/elements[i].subelements[j].length()) * tempMat;
            //printInfoToDebug(elements[i].subelements[j].K);
        }
    }
}

//Creates a global stiffness matrix K.
arma::mat createGlobalStiffnessMatrix(std::vector<element> &elements, const std::vector<node> nodes, int stiffnessSize){
    //Code element order is nodes[0].codeX, nodes[0].codeY, nodes[0].codeZ, nodes[1].codeX, nodes[1].codeY, nodes[1].codeZ.
    //Number of nodes == n.  Then K will be nxn.
    //We initalize our matrix K.
    arma::mat tempMat;
    tempMat.zeros(stiffnessSize, stiffnessSize);
    for (size_t i = 0; i < elements.size(); i++){
        for (size_t j = 0; j < elements[i].subelements.size(); j++){
            //We are now at the subelement level.
            //We create a map of the code names.
            int codeMap[6][6][2];
            //We populate the code map.
            //Populate the second element of matrix of 1x2 elements.
            /*
            qDebug()<<"x1" << elements[i].subelements[j].nodes[0].codeX;
            qDebug()<<"y1" << elements[i].subelements[j].nodes[0].codeY;
            qDebug()<<"z1" << elements[i].subelements[j].nodes[0].codeZ;
            qDebug()<<"x2" << elements[i].subelements[j].nodes[1].codeX;
            qDebug()<<"y2" << elements[i].subelements[j].nodes[1].codeY;
            qDebug()<<"z2" << elements[i].subelements[j].nodes[1].codeZ;
            /**/
            for (int k = 0; k < 6; k++){
                codeMap[k][0][1] = elements[i].subelements[j].nodes[0].codeX;
                codeMap[k][1][1] = elements[i].subelements[j].nodes[0].codeY;
                codeMap[k][2][1] = elements[i].subelements[j].nodes[0].codeZ;
                codeMap[k][3][1] = elements[i].subelements[j].nodes[1].codeX;
                codeMap[k][4][1] = elements[i].subelements[j].nodes[1].codeY;
                codeMap[k][5][1] = elements[i].subelements[j].nodes[1].codeZ;
            }
            for (int k = 0; k < 6; k++){
                codeMap[0][k][0] = elements[i].subelements[j].nodes[0].codeX;
                codeMap[1][k][0] = elements[i].subelements[j].nodes[0].codeY;
                codeMap[2][k][0] = elements[i].subelements[j].nodes[0].codeZ;
                codeMap[3][k][0] = elements[i].subelements[j].nodes[1].codeX;
                codeMap[4][k][0] = elements[i].subelements[j].nodes[1].codeY;
                codeMap[5][k][0] = elements[i].subelements[j].nodes[1].codeZ;
            }
            //The code map is now populated.  We can now use it to map the member stiffness k's to the global K.
            //We cycle through each member of the member stiffness k, and add it to the proper position of global K.
            for (int x = 0; x < 6; x++){
                //QString debuggingString;
                for (int y = 0; y < 6; y++){
                    /*
                    debuggingString += "[";
                    debuggingString += QString::fromStdString(std::to_string(codeMap[x][y][0]));
                    debuggingString += ", ";
                    debuggingString += QString::fromStdString(std::to_string(codeMap[x][y][1]));
                    debuggingString += "] ";
                    /**/
                    //Note that for the global stiffness matrix the code 1, 1 actually denotes index 0, 0.  So we subtract 1 to account for starting at 0.
                    int row = codeMap[x][y][0] - 1;
                    int col = codeMap[x][y][1] - 1;
                    //qDebug()<<row;
                    //qDebug()<<col;
                    //Don't do anything to constrained things.
                    if (row == -1 || col == -1){
                        continue;
                    }
                    tempMat(row, col) += elements[i].subelements[j].K(x, y);
                }
                //qDebug()<<debuggingString;
            }
        }
    }
    return tempMat;
}
