#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "conjgrad.h"
#include "cdt_bords.h"
#include "assert.h"
#include "decomposition.h"
#include "operator_matrix.h"

using namespace std;

void nloc(int *i, int *j, int n, int Nx);

void RightHandSide(int N, int Nx, int M, double dx, double dy,
				   double Cx, double Cy,double *RHS);

int main(){
  int Nx = 10,Ny =10,N = Nx*Ny, maxiter=1000;
	double 	Lx = 1.,  Ly = 1., D =1., eps = 10e-7;
	decomposition Dc(0, 1, 1, Nx, Ny);
	operator_matrix A(Nx, Ny, Lx, Ly, D);
					
  double *U,*Uold,*RHS;
	U    = (double*) calloc(N,sizeof(double)); 
  Uold = (double*) calloc(N,sizeof(double));
  RHS  = (double*) calloc(N,sizeof(double));
	//RightHandSide(N, Nx, Ny, dx, dy, Cx, Cy, RHS);
	for(int i=0;i<N;++i){
		RHS[i] = 10;
		U[i] = 1.1;
	}
	/*int *idx = Dc.get_index_global();
	for(int i=0;i<Nx;++i){
		for(int j=0;j<Ny;++j)
			cout << idx[i+j*(Nx)] <<" ";
		cout << endl;
	}*/
	CGmethod CG(A,&Dc,RHS,U);
	CG.compute(maxiter,eps);
	CG.save();
}

int cdt_choisie= 1;
cdt_aux_bords cdt[] = 
{
  {f2, func_zero, func_zero},
  {f1, f1, f1}
};

void RightHandSide(int N, int Nx, int M, double dx, double dy, double Cx, double Cy,double *RHS)
{
  int i,j,l,k;
  double posx,posy;

  //M = N/Nx ; /* # de lignes */
  assert(N/Nx == M);
  double (*f)(double, double, double) = cdt[cdt_choisie].f;
  double (*g)(double, double, double) = cdt[cdt_choisie].g;
  double (*h)(double, double, double) = cdt[cdt_choisie].h;

  for( i = 1; i<=N; i++ ){
    nloc(&j,&k,i,Nx);
    posx = k*dx;
    posy = j*dy;
    RHS[i] = f(posx,posy,0.0);
  }

  /* premiere ligne condition de bord du bas */
  
  for( i = 1; i<= Nx; i++ ){
  nloc(&j,&k,i,Nx);
  posx = k*dx;
  posy = j*dy;
  RHS[i] = RHS[i]-g(posx,0.0,0.0)*Cy;
      }

  /* derniere ligne condition de bord du haut */
  l = 1;
  for( i = N-Nx+1;i<=N;i++ ){ 
      nloc(&j,&k,i,Nx);
      posx = k*dx;
      posy = j*dy;
      RHS[i] =RHS[i]-g(posx,1.0,0.0)*Cy;
    }

  /* Bords droit et gauche */
    /*Ligne du bas*/
  RHS[1]  = RHS[1]  -h(0.0,dy,0.0)*Cx;
  RHS[Nx] = RHS[Nx] -h(1.0,dy,0.0)*Cx;

  /*Ligne du milieux*/
  j = 1+Nx;
  for( i = 2; i<= M-1; i++ ){
    nloc(&k,&l,j,Nx);
    RHS[j] = RHS[j] -h(0.0,k*dy,0.0)*Cx;
    RHS[j+Nx-1] = RHS[j+Nx-1] -h(1.0,k*dy,0.0)*Cx;
    j = 1 + (i)*Nx;
  }
  /*ligne du haut*/
  nloc(&k,&l,N,Nx);
  RHS[N-Nx+1] = RHS[N-Nx+1] -h(0.0,k*dy,0.0)*Cx;
  RHS[N] = RHS[N] -h(1.0,k*dy,0.0)*Cx;
}

void nloc(int *i, int *j, int n, int Nx)
{
  int q,r;
  q = n/Nx;
  r = n - q*Nx;  
  if ( r == 0 ){
    *i = q;
    *j = Nx;
  }
  else{
    *i = 1+q;
    *j = r;
  }
  return;
}