#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <mpi.h>

#include "conjgrad.h"
#include "decomposition.h"
#include "operator_matrix.h"
#include "RHS.h"

using namespace std;

int main(){
	int myrank, n_procs;
	MPI_Init(NULL,NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

	int Nx = 20,Ny =20,N = Nx*Ny, maxiter=99;
	double 	Lx = 1.,  Ly = 1., D =1., eps = 10e-10;
	decomposition Dc(myrank, n_procs, 1, Nx, Ny);
	operator_matrix A(Nx, Ny, Lx, Ly, D,false);

	double *U,*RHS;
	U  = (double*) calloc(N,sizeof(double));
	RHS  = (double*) calloc(N,sizeof(double));
	for(int i=0;i<N;++i) U[i] = 1;
	
	fill_RHS_force(&Dc,&A,RHS,&one);
	//fill_RHS_NeumannBC(&Dc,&A,RHS);
	fill_RHS_DirichletBC(&Dc,&A,RHS,&null);
	CGMethod CG(&A,&Dc,RHS,U);
	CG.compute(maxiter,eps);
	CG.save();
	MPI_Finalize();
}