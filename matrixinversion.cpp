#include "matrixinversion.h"
//https://github.com/ZhengzhongSun/Matrix-Inversion-with-CUDA/blob/master/matrixInversion_cpu.cpp
#include<stdio.h>
#include<math.h>
#include<iostream>
#include<fstream>
#include<time.h>

//Sets how accurate we want to be.
#define minvalue 0.00005

int i, j, k, temp;     /* declaring counter variables for loops */

using namespace std;

/* using gauss-jordan elimination */
//Returns the inverse matrix.
arma::mat gauss_jordan(arma::mat armaM){
    float temporary, r;

    int dimension = armaM.n_cols;
    float augmentedmatrix[dimension][2 * dimension];

    //We concatenate the matrices horizontally.
    arma::mat identity;
    identity.eye(dimension, dimension);
    arma::mat merged = arma::join_rows(armaM, identity);

    for (i = 0; i < dimension; i++){
        for (j = 0; j < 2*dimension; j++){
            augmentedmatrix[i][j] = merged(i,j);
        }
    }

    //We have now joined the original and arma I think.

    for (j = 0; j<dimension; j++){
        temp = j;

        /* finding maximum jth column element in last (dimension-j) rows */

        for (i = j + 1; i<dimension; i++)
        if (augmentedmatrix[i][j]>augmentedmatrix[temp][j])
            temp = i;

        if (fabs(augmentedmatrix[temp][j])<minvalue){
            //printf("\nElements are too small to deal with.");
            break;
        }

        /* swapping row which has maximum jth column element */

        if (temp != j)
        for (k = 0; k<2 * dimension; k++){
            temporary = augmentedmatrix[j][k];
            augmentedmatrix[j][k] = augmentedmatrix[temp][k];
            augmentedmatrix[temp][k] = temporary;
        }

        /* performing row operations to form required identity matrix out of the input matrix */

        for (i = 0; i<dimension; i++)
        if (i != j){
            r = augmentedmatrix[i][j];
            for (k = 0; k<2 * dimension; k++)
                augmentedmatrix[i][k] -= (augmentedmatrix[j][k] / augmentedmatrix[j][j])*r;
        }
        else{
            r = augmentedmatrix[i][j];
            for (k = 0; k<2 * dimension; k++)
                augmentedmatrix[i][k] /= r;
        }

    }

    arma::mat toBeReturned;
    toBeReturned.zeros(dimension, dimension);
    for (i = 0; i < dimension; i++){
        for (j = 0; j < dimension; j++){
            toBeReturned(i,j) = augmentedmatrix[i][j];
        }
    }

    return toBeReturned;
}

