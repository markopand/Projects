#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#define GG 20
#define DG 0
#include<math.h>
void input_matrix( FILE *pf, double **M, int rows,int colums)
{
  int i,j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < colums; j++) {
      fscanf(pf, "%lf", &M[i][j]);
    }
  }  
}

void print_matrix(double **M, int rows,int colums) 
{
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < colums; j++) {
      printf("%lf\t", M[i][j]);
      
    }
    printf("\n");
  }   
}

double** allocate_matrix(int rows,int colums) 
{  
  double **M;
  int i;
  
  M = (double**) malloc ( rows* sizeof(double*) );
  for (i = 0; i < rows; i++) {
    M[i] = (double*) malloc ( colums * sizeof(double));
  }
  return M;
}

void make_diag_matrix(double **M,int n)
{
int i,j;

for(i=0;i<n;i++){
	for(j=0;j<n;j++){
		if(i==j){
		M[i][j]=1;
		}
		else M[i][j]=0;
	}
}	
}

void gem(double **A,double **L,int n,int k)
{
    /*Gaussian elimination
    * Computation of coefficient ratio between rows stored in matrix L
    * Reduction of rows starting from row=k+1,column=k in matrix A*/
    int i,j;
    #pragma omp parallel for shared(L,A) private(j,i) schedule(guided)
    for(i=k+1;i<=n-1;i++){ // there are no dependencies between two for loops
              L[i][k]=A[i][k]/A[k][k]; 
              for(j=k;j<=n-1;j++){
                  A[i][j]=A[i][j]-L[i][k]*A[k][j]; //This reduction cannot be parallelised because it is matrix, if it were an array it would be possible to make a reduction
              }
            }
}

void swap(double **A,int k,int l,int index)
{
	//swaps two values
    double temp;
	temp=A[k][l];
    A[k][l]=A[index][l];
    A[index][l]=temp;
	
}
void pivoting(double **A,double **P,double **L,int n)
{
    /*Pivoting and Gaussian elimination
    *Searching for j>k such that |a(j)(k)|>|a(k)(k)|
    *If j exists, switch A(j)(:)<->A(k)(:), P(j)(:)<->P(k)(:) and L(j)(0:k-1)<->L(k)(0:k-1)
    *Then we call the function gem that does the gauss elimination on column k */
    int j,k,index,l;
    double max;
    for(k=0;k<n-1;k++){ // k loop cannot be parallelised because of dependencies
         max= fabs(A[k][k]);  // if k was parallelised max and index could be made into reduction as below
         index=k;
	    #pragma omp parallel for shared(A,k) private(j) schedule(guided) reduction(max:max,index)
        for(j=k+1;j<=n-1;j++){
            if(fabs(A[j][k])>max){
                max=fabs(A[j][k]); // this is parallelizable, because each thread does partial reduction of a max
                index=j;
            }
           
           
        }
        if(index!=k){
            #pragma omp parallel for shared(A,P,L,n,k,index) schedule(guided)
            for( l=0;l<=n-1;l++){
            	swap(A,k,l,index);
                swap(P,k,l,index);
              if(l<k)
            	swap(L,k,l,index);
                }
            
            
		 }
		 gem(A,L,n,k);
	}
}
void forwardsub(double **P,double **L,int n)
{
    /*Algorithm for solving linear system that involves a lower triangular matrix-> Ly=P
    *The computation starts from the row 0 and uses L(0)(:),P(0)(:) to obtain y(0)(:)
    *Iterative variable k goes from 0 to n-2
    *Then iteration k uses L(k)(:),P(k)(:),y(0:k-1) to obtain y(k)(:)
    */
	int i,j,k;
	//#pragma omp parallel for shared(L,P,n) schedule(static,2) private(i,j,k)
    //for(j=0;j<n;j++){
	 for(k=0;k<=n-2;k++){ // k loop cannot be parallelised because of dependencies
            #pragma omp parallel for shared(L,P,n,k) schedule(guided) private(i,j)
	 	     for(i=k+1;i<=n-1;i++){
                for(j=0;j<n;j++){	             
                    P[i][j]-=L[i][k]*P[k][j];  // this is a reduction, however, reduction is not possible on matrices
                }
            }
        }
}
void backwardsub(double **A,double **P,int n)
{
    /*Algorithm for solving linear system that involves an upper triangular matrix-> Ux=P
    *The computation starts from the row n and uses L(n)(:),P(n)(:) to obtain y(n)(:)
    *Iterative variable k goes from n-1 to 0
    *Then iteration k uses L(k)(:),P(k)(:),y(k+1:n) to obtain y(k)(:)
    */
	int i,j,k;
	#pragma omp parallel for shared(A,P,n) schedule(guided) private(j,k,i) // there are no dependencies
	for(j=0;j<n;j++){
       P[n-1][j]/=A[n-1][n-1]; 
            for(k=n-2;k>=0;k--){
                for(i=k+1;i<=n-1;i++){
                    P[k][j]-=A[k][i]*P[i][j]; // this is a reduction, however, reduction is not possible on matrices
                }
                P[k][j]/=A[k][k];  
            }
    }
}
void release_mem(double **M,int rows)
{
int i;

for(i=0;i<rows;i++)
free(M[i]);
free(M);
}

void prandom(double **M,int rows,int columns)
{
	int i,j;
	
	srand ((unsigned)time(NULL ));
	for(i=0;i<rows;i++){
		for(j=0;j<columns;j++){
			M[i][j]=DG+ (float)rand () / RAND_MAX * (GG - DG);

		}
	}
	
}
double** matrix_multi(double **M,double **A,int n,int m,int k)
{
	double **C;
	int i,j,p;
	//The most common algorithm for matrix multiplication where (n x m) (m x k) = (n x k)
	C=allocate_matrix(n,k);
	#pragma omp parallel for if(n*m>7000) private(i,j,p)
	for(i=0; i<n; i++){
		for(j=0; j<k; j++){
			C[i][j]=0;
			for(p=0; p<m; p++)
				C[i][j] += M[i][p] * A[p][j];}}
return C;
}
void process1(int rows)
{
  double **L, **P,**A,a,b;
  int i, cols;
  cols=rows;
  A=allocate_matrix(rows,cols);
  P=allocate_matrix(rows,cols);
  L=allocate_matrix(rows,cols);
  prandom(A,rows,cols);
  make_diag_matrix(P,rows);
  make_diag_matrix(L,rows);
  a=omp_get_wtime();
   pivoting(A,P,L,rows);
  forwardsub(P,L,rows);
  backwardsub(A,P,rows);
  b=omp_get_wtime();
  printf("Time elapsed: %lf\n",b-a);
  release_mem(P,rows);
  release_mem(A,rows);
  release_mem(L,rows);
}

void process2(int rows)
{
  double **L, **P,**A,a,b;
  int i, cols;
  cols=rows;
  A=allocate_matrix(rows,cols);
  P=allocate_matrix(rows,cols);
  L=allocate_matrix(rows,cols);
  prandom(A,rows,cols);
  prandom(P,rows,cols);
  a=omp_get_wtime();
  matrix_multi(A,P,rows,cols,rows);
  b=omp_get_wtime();
  printf("Time elapsed: %lf\n",b-a);
  release_mem(P,rows);
  release_mem(A,rows);
  release_mem(L,rows);
}

int main(void)
{
	int dim,th;
	printf("Matrix Inverse : \n\n");
    for(dim=500;dim<5001;dim+=500){
        printf("Dimension: %d\n",dim);
          for(th=2;th<=24;th+=2){
            omp_set_num_threads(th);
            printf("Threads: %d\n",th);
            process1(dim);
        }
    }  
    printf("Matrix Multiplication : \n\n");
    for(dim=500;dim<5001;dim+=500){
        printf("Dimension: %d\n",dim);
          for(th=2;th<=24;th+=2){
            omp_set_num_threads(th);
            printf("Threads: %d\n",th);
            process2(dim);
        }
    }  
}




