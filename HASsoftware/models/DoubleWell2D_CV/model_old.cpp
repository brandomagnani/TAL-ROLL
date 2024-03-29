//
//  Model.cpp
//
//  model name: 2D Double Well with Extended Variable
//
//  Adapted from Jonathan Goodman's foliation sampler code.
//
/*
     Enter description
*/
#include <iostream>
#include <blaze/Math.h>
#include "model.hpp"
using namespace std;
using namespace blaze;

// DEFAULT Constructor
Model::Model(){}

// PARAMETRIZED Constructor, copy given dimensions into instance variables
Model::Model( int                         d0,    /* dimension of the ambient space */
              int                         m0,    /* number of constraints          */
            // double well potential parameters:
              double D00,
              double a0,
              double kappa0,
              double lambda0 ){
   
   // building the model object
   d      = d0;
   m      = m0;
   D0     = D00;
   a      = a0;
   kappa  = kappa0;
   lambda = lambda0;
   
}

// COPY Constructor
Model::Model(const Model& M0){
   d      = M0.d;
   m      = M0.m;
   D0     = M0.D0;
   a      = M0.a;
   kappa  = M0.kappa;
   lambda = M0.lambda;
}


double
Model::V(DynamicVector<double, columnVector> q) {
   
   double V = D0 * (q[0]*q[0] - a*a) * (q[0]*q[0] - a*a) + 0.5 * kappa * q[1]*q[1] + lambda * q[0] * q[1];
   
   return V;
}

DynamicVector<double, columnVector>
Model::gV(DynamicVector<double, columnVector> q){
   
   DynamicVector<double, columnVector> gV(d);
   
   gV[0] = 4. * D0 * (q[0]*q[0] - a*a) * q[0] + lambda * q[1];
   gV[1] = kappa * q[1] + lambda * q[0];
   gV[2] = 0.;
   
   return gV;
}



DynamicVector<double, columnVector>
Model::xi(DynamicVector<double, columnVector> q){
   
   DynamicVector<double, columnVector> xi(m);      // constraint function values
   xi[0] = q[0] - q[2];
   
   return xi;
}


DynamicMatrix<double, columnMajor>
Model::gxi(DynamicVector<double, columnVector> q){
   
   DynamicMatrix<double, columnMajor> gxi(d,m);  // gradient, (dxm) matrix
   gxi(0, 0) = 1.;
   gxi(1, 0) = 0.;
   gxi(2, 0) = -1.;
   
   return gxi;
}


// Returns gxi(q) augmented to a square matrix (in case d > m), last n=d-m columns are just zeros.
// Needed to have a complete QR decomposition of gxi(q), with Q (dxd) matrix
DynamicMatrix<double, columnMajor>
Model::Agxi(DynamicMatrix<double, columnMajor>& gxi){
   
   DynamicMatrix<double, columnMajor> Agxi(d,d);   // Augmented gradient, (dxd) matrix
   Agxi = 0.;  // initialize to zero
   
   for (int j = 0; j < m; j++){
      column(Agxi, j)  = column(gxi, j);
   }

   return Agxi;  // last n=d-m columns are zeros
}


// Multiplies the top d-m rows of gxi by c1 and the bottom m rows by c2
DynamicMatrix<double, columnMajor>
Model::scaled_gxi(const DynamicMatrix<double, columnMajor>& gxi, double c1, double c2) {

    // Create a copy of the input matrix to hold the results
    DynamicMatrix<double, columnMajor> scaled_gxi = gxi;

    // Make sure that d is greater than m
    if(d <= m) {
        cout << " The number of rows d must be greater than m! " << endl;
        return scaled_gxi; // Return the unchanged copy in case of error
    }
   
    // Multiply the top d-m rows by c1
    auto top = submatrix(scaled_gxi, 0, 0, d-m, m);
    top *= c1;

    // Multiply the bottom m rows by c2
    auto bottom = submatrix(scaled_gxi, d-m, 0, m, m);
    bottom *= c2;

    return scaled_gxi;
}


string Model::ModelName(){                  /* Return the name of the model */
   return(" 2D Double Well with Extended Variable ");
}

 
double Model::yzIntegrate( double x, double L, double R, double eps, int n){

//  Use the rectangle rule in the y and z directions with n midpoints in each direction

   double dy  = (R-L)/( (double) n);
   double dz  = dy;
   double y, z;
   double sum = 0.;
   
   DynamicVector<double, columnVector> qv( d);    // point in 3D
   DynamicVector<double, columnVector> xiv( m);   // values of the constraint functions
   double Vqv( d);                                // values of potential
   
   qv[0] = x;
   for ( int j = 0; j < n; j++){
      y = L + j*dy + .5*dy;
      qv[1] = y;
      for ( int k = 0; k < n; k++) {
         z = L + k*dz + .5*dz;
         qv[2] = z;
         xiv = xi(qv);
         Vqv = V(qv);
         sum += exp( - Vqv - 0.5*(trans(xiv)*xiv)/(eps*eps) );
      }
   }
   return dy*dz*sum;
}
