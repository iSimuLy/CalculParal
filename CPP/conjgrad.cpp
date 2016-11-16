#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "conjgrad.h"
#include "tools.h"

// constructor
CGmethod::CGmethod(double aii,double cx,double cy,int nx,int n,double *rhs,double *u){
	Aii = aii; Cx = cx; Cy = cy; Nx = nx; N = n; RHS = rhs; U = u;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_procs);
}

/* solveur du gradient conjugue */
void CGmethod::compute(int iterMax, double eps){
  double residue, drl, dwl, alpha, beta;
  double *r, *kappa, *d, *W;
  MPI_Status s1, s2;
  MPI_Request r1, r2, r3, r4;

  iter = 0;
  Ny = N/Nx;
  residue = 0.0;

  r     = (double*) calloc(N+1,sizeof(double)); 
  kappa = (double*) calloc(N+1,sizeof(double));
  d     = (double*) calloc(N+1,sizeof(double));
  W     = (double*) calloc(N+1,sizeof(double));

  int fstline = myrank*Ny/nb_procs+1;
  int lstline = (myrank+1)*Ny/nb_procs;
  if(myrank == nb_procs-1)
    lstline = Ny;

  int fst = (fstline-1) * Nx + 1;
  int lst = lstline * Nx;

  int deb = MAX(fst-Nx, 1);
  int fin = MIN(lst+Nx,N);

// Initialisation des variables pour le calcul du Gradient Conjugué.
  for (int i=deb;i<fin;i++ ){
    kappa[i] = U[i];
  }
  matvec(Aii,Cx,Cy,Nx,Ny,kappa,r);

  for(int i=fst;i<=lst;i++){
    r[i]     = r[i] - RHS[i];
    residue = residue + r[i]*r[i];
    d[i]=r[i];
  }
// Réduction pour que tout le monde ait la bonne valeur du résidu.
  MPI_Allreduce(MPI_IN_PLACE, &residue, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
/* boucle du Gradient conjugue */
  while( (iter<=iterMax) && (sqrt(residue) >= eps)){    
// Communication des parties dont j'ai besoin.
    if(myrank != nb_procs - 1){
      MPI_Isend(&d[lst - Nx + 1], Nx, MPI_DOUBLE,myrank+1, 99, MPI_COMM_WORLD, &r1);
      MPI_Irecv(&d[lst+1], Nx, MPI_DOUBLE,myrank+1, 99, MPI_COMM_WORLD, &r2);
    }
    if(myrank != 0){
      MPI_Isend(&d[fst], Nx, MPI_DOUBLE, myrank-1, 99, MPI_COMM_WORLD, &r3);
      MPI_Irecv(&d[fst-Nx], Nx, MPI_DOUBLE, myrank-1, 99, MPI_COMM_WORLD, &r4);
    }
    if(myrank != nb_procs - 1)
    {
      MPI_Wait(&r1, &s1);
      MPI_Wait(&r2, &s1);
    }
    if(myrank != 0)
    {
      MPI_Wait(&r3, &s2);
      MPI_Wait(&r4, &s2);
    }

// W = A*d
    matvec(Aii,Cx,Cy,Nx,Ny,d,W);
    drl = 0.0;
    dwl = 0.0;
    for(int i=fst; i<=lst; i++ ){
      drl += d[i]*r[i];
      dwl += d[i]*W[i];
    }
    MPI_Allreduce(MPI_IN_PLACE, &dwl, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(MPI_IN_PLACE, &drl, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    alpha = drl/dwl;
    for(int i=fst; i<=lst; i++ ){
      kappa[i] = kappa[i] - alpha*d[i];
      r[i] = r[i] - alpha*W[i];
    }
    beta = 0.0;
    for(int i=fst;i<=lst;i++){
      beta = beta + (r[i]*r[i]);
    }
    MPI_Allreduce(MPI_IN_PLACE, &beta, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    beta = beta / residue;
    residue = 0.0;
    for(int i = fst; i<= lst; i++){
      d[i] = r[i] + beta*d[i];   
      residue    = residue + r[i]*r[i];  
    }
    MPI_Allreduce(MPI_IN_PLACE, &residue, 1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    iter++;
  }
// Recopie de la solution dans U 
  for(int i=fst;i<=lst;i++){
    U[i] = kappa[i]; 
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if(myrank == 0){
    printf("le Gradient Conjugue a converge en"
           " %d iteration, residue= %0.12f\n", iter,residue);
  }
	free(r); free(kappa); free(d); free(W);
}
