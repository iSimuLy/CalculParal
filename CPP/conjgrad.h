#ifndef CONJGRAD_H
#define CONJGRAD_H

class CGmethod{
	CGmethod();
	CGmethod(double Aii,double Cx,double Cy,int Nx,int N,double *RHS,double *U);
	~CGmethod();
public:
	void compute(int iterMax, double eps);
private:
	// system variables
	double Aii,Cx,Cy, eps;
	double *RHS, *U;
	int iterMax,iter, Nx,Ny,N;
	// MPI variables
  int myrank, nb_procs;
	// functions
	void init();
	void main_workload(int itermax, double e);
	void cleanup();
};


#endif