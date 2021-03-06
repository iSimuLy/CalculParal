#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <mpi.h>
#include "decomposition.h"

using namespace std;

void print_D(decomposition *D, int myRank);
void print_D_inner(decomposition *D, int myRank);
void print_D_col(decomposition *D, int myRank);
void print_D_row(decomposition *D, int myRank);

int main(){
	int myRank, nOfProcs, nOfProcs_x=nOfProcs, nx=10, ny=20, overlap=1;
	MPI_Init(NULL,NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &nOfProcs);
	
	decomposition D(myRank, nOfProcs, 1, nx, ny,overlap);
	for(int k=0;k<nOfProcs;++k){
		if(myRank==k){
			print_D(&D,myRank);
			print_D_row(&D,myRank);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
	return 0;
}

void print_D(decomposition *D, int myRank){
	int* index = D->get_index_global(), 
				myNx=D->get_myNx(), myNy=D->get_myNy();
	//cout << "#" << myRank << ". " << myNx << " " << myNy;
	for(int i=0;i<myNy;++i){
		cout << "#" << myRank << ". ";	
		for(int j=0;j<myNx;++j)
			cout << index[j+myNx*i] << " ";
		cout << endl;
	}
}

void print_D_inner(decomposition *D, int myRank){
	int* index = D->get_index_global_inner(), 
				myNx=D->get_myNx(), myNy=D->get_myNy();
	//cout << "#" << myRank << ". " << myNx << " " << myNy;
	for(int i=0;i<myNy-2;++i){
		cout << "#" << myRank << ". ";	
		for(int j=0;j<myNx-2;++j)
			cout << index[j+(myNx-2)*i] << " ";
		cout << endl;
	}
}

void print_D_col(decomposition *D, int myRank){
	int* index = D->get_index_global_msg_left(), 
			 myNy=D->get_myNy();
	//cout << "#" << myRank << ". " << myNx << " " << myNy;
	for(int i=0;i<myNy;++i){
		cout << "#" << myRank << ". ";	
		cout << index[i] << endl;
	}
}

void print_D_row(decomposition *D, int myRank){
	int* index = D->get_index_global_msg_top(), 
			 myNx=D->get_myNx();
	//cout << "#" << myRank << ". " << myNx << " " << myNy;
	cout <<endl<< "#" << myRank << ". ";	
	for(int i=0;i<myNx;++i){
		cout << index[i] << " ";
	}
	cout << endl<<endl;
}