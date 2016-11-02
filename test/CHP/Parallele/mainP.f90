program parallele
  use fonctions
  use grad
  !use fonctions
  !use MPI
  implicit none
  include "mpif.h"
  integer::Nx=3,Ny=3
  integer::Me,statinfo,tag,i,j,i1,iN,Np,k
  integer,dimension(MPI_STATUS_SIZE)::status
  real*8::B,Cx,Cy,D,Lx,Ly,dt,dx,dy
  real*8,dimension(:),allocatable::U,U0,X,Y,Uexacte,RHS,f,g,h
  character(len=20)::parametres='param.txt'

  call MPI_INIT(statinfo)
  call MPI_COMM_RANK(MPI_COMM_WORLD,Me,statinfo)
  tag=100
  call MPI_COMM_SIZE(MPI_COMM_WORLD,Np,statinfo)

  open (unit = 10, file =parametres , form = 'formatted', status = 'old', action = 'read')
  read(10,*)Nx,Ny,Lx,Ly,D
  close(10)

  call charge(Nx*Ny,Np,me,i1,iN)

  allocate(U(Nx*Ny))
  allocate(U0(Nx*Ny))
  allocate(X(Nx))
  allocate(Y(Ny))
  allocate(Uexacte(Nx*Ny))
  allocate(RHS(Nx*Ny))
  allocate(f(Nx*Ny))
  allocate(g(2*Nx))
  allocate(h(2*Ny))

  !stationnaire
  dx=Lx/(1+Nx)
  dy=Ly/(1+Ny)
  Cx=-D/dx**2
  Cy=-D/dy**2
  B=2*D/dx**2+2*D/dy**2
  U=0.
  call Init(X,Y,dx,dy,Nx,Ny)

  !trio 1
  f=f1(X,Y)
  g=g1(X,Y)
  h=h1(X,Y)
  do i=1,Nx
     do j=1,Ny
        Uexacte((j-1)*Nx+i)=X(i)*(Lx-X(i))*Y(j)*(Ly-Y(j))
     end do
  end do

  RHS=U+f
  call CalculRHS(Cx,Cy,h(:),g(:),RHS(:))
  call GradConj(Nx,Ny,B,Cx,Cy,U(:),RHS(:))


  deallocate(U)
  deallocate(U0)
  deallocate(X)
  deallocate(Y)
  deallocate(Uexacte)
  deallocate(RHS)
  deallocate(f)
  deallocate(g)
  deallocate(h)




  call MPI_FINALIZE(statinfo)

contains

  subroutine charge(n,Np,me,i1,iN)
    implicit none
    integer, intent (in)::n,Np,me
    integer, intent (out)::i1,iN
    integer::q
    q=n/Np
    if(me<mod(n,Np))then
       i1=me*(q+1)+1
       iN=(me+1)*(1+q)
    else
       i1=mod(n,Np)+1+me*q
       iN=i1+q-1
    end if
  end subroutine charge

  subroutine Rename(me,name)
    implicit none
    integer,intent(in)::me
    character*13::name
    character*3::tn
    integer::i1,i2,i3
    i1=me/100
    i2=(me-100*i1)/10
    i3=me-100*i1-10*i2
    tn=char(i1+48)//char(i2+48)//char(i3+48)
    name='sol'//tn//'.dat'
  end subroutine Rename

end program parallele
