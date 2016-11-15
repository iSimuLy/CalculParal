#include "conjgrad.h"
#include "tools.h"


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* solveur du gradient conjugue */
void GC(int maxiter, double eps, double Aii,double Cx,double Cy,
  int Nx,int N,double *RHS,double *U)
{
  int myrank, nb_procs;
  int l, i, M;
  double residu, drl, dwl, alpha, beta;
  double *r, *kappa, *d, *W;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_procs);
  MPI_Status s1, s2;
  MPI_Request r1, r2, r3, r4;

  l = 0;
  M = N/Nx;
  residu = 0.0;
// +1 car je commence a 1 pour compatibilite fortran
  r     = (double*) calloc(N+1,sizeof(double)); 
  kappa = (double*) calloc(N+1,sizeof(double));
  d     = (double*) calloc(N+1,sizeof(double));
  W     = (double*) calloc(N+1,sizeof(double));

  int fstline = myrank*M/nb_procs+1;
  int lstline = (myrank+1)*M/nb_procs;
  if(myrank == nb_procs-1)
    lstline = M;

  int fst = (fstline-1) * Nx + 1;
  int lst = lstline * Nx;

  int deb = MAX(fst-Nx, 1);
  int fin = MIN(lst+Nx,N);

// Initialisation des variables pour le calcul du Gradient Conjugué.
  for ( i=deb;i<fin;i++ ){
    kappa[i] = U[i];
  }
  matvec(Aii,Cx,Cy,Nx,M,kappa,r);

  for(i=fst;i<=lst;i++){
    r[i]     = r[i] - RHS[i];
    residu = residu + r[i]*r[i];
    d[i]=r[i];
  }
// Réduction pour que tout le monde ait la bonne valeur du résidu.
  MPI_Allreduce(MPI_IN_PLACE, &residu, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
/* boucle du Gradient conjugue */
  while( (l<=maxiter) && (sqrt(residu) >= eps)){    
// Communication des parties de d dont j'ai besoin.
    if(myrank != nb_procs - 1){
      MPI_Isend(&d[lst - Nx + 1], Nx, MPI_DOUBLE,
       myrank+1, 99, MPI_COMM_WORLD, &r1);
      MPI_Irecv(&d[lst+1], Nx, MPI_DOUBLE,
       myrank+1, 99, MPI_COMM_WORLD, &r2);
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
    matvec(Aii,Cx,Cy,Nx,M,d,W);
    drl = 0.0;
    dwl = 0.0;
    for( i=fst; i<=lst; i++ ){
      drl += d[i]*r[i];
      dwl += d[i]*W[i];
    }
    MPI_Allreduce(MPI_IN_PLACE, &dwl, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(MPI_IN_PLACE, &drl, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    alpha = drl/dwl;
    for(i=fst; i<=lst; i++ ){
      kappa[i] = kappa[i] - alpha*d[i];
      r[i] = r[i] - alpha*W[i];
    }
    beta = 0.0;
    for(i=fst;i<=lst;i++){
      beta = beta + (r[i]*r[i]);
    }
    MPI_Allreduce(MPI_IN_PLACE, &beta, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    beta = beta / residu;
    residu = 0.0;
    for( i = fst; i<= lst; i++){
      d[i] = r[i] + beta*d[i];   
      residu    = residu + r[i]*r[i];  
    }
    MPI_Allreduce(MPI_IN_PLACE, &residu, 1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    l++;
  }
// Recopie de la solution dans U 
  for(i=fst;i<=lst;i++){
    U[i] = kappa[i]; 
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if(myrank == 0){
    printf("le Gradient Conjugue a converge en"
           " %d iteration, residu= %0.12f\n", l,residu);
  }
}

