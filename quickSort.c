#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void swap(double*, double*);
void partition_array(double, int);
void print_array();
void quick_sort(double *A, int A_size, int R);


int count = 1;
int main(int argc, char *argv[]) {

  int i, arr_size, j, rank, size;
  double *array, *array_local;
  double pivot, a;


  MPI_Datatype strided, arr;
  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */
  MPI_Status status[size];
  MPI_Request request;
  arr_size=10;
  array_local = malloc(arr_size*sizeof(double));

  int recv_counts[size];
  int recv_displ[size];
  if(rank == 0)
    { 

      array = malloc(arr_size*sizeof(double)); //Initialize test array
      for(i=0;i<arr_size;i++){
	//	array[i] =rand()%20;
	array_local[i] = rand()%20;
      }
      print_array(array_local, arr_size, rank);
    }
  MPI_Barrier(MPI_COMM_WORLD);
  while (count < size){
    
    if (rank < count && rank + count < size){
      //partition array according to pivot
      pivot = array_local[1];
      printf("rank: %d pivot = %g arr_size=%d\n",rank, pivot, arr_size);
      for (i = 0, j = arr_size - 1; ; i++, j--)
	{
	  //	  if (i >= j) break;
	  while (array_local[i] < pivot){
	    printf("\n proc %d in i-while\n", rank);
	    i++;}
	  while (array_local[j] > pivot)
	    {printf("\n proc %d in j-while\n", rank);
	      j--;
	    }
	  if (i >= j) break;
 
	  swap(&array_local[i], &array_local[j]);
	}     
      print_array(array_local, arr_size, rank);
      printf("\nrank:%d i = %d j = %d\n ",rank, i, j);
      MPI_Type_vector(1, arr_size-i, arr_size-i, MPI_DOUBLE, &strided);
      MPI_Type_commit(&strided);
      MPI_Send(&array_local[i], 1, strided, rank + count, 111, MPI_COMM_WORLD);   
      printf("\nrank:%d did send\n ",rank);
      arr_size = i;
      MPI_Barrier(MPI_COMM_WORLD);

    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank >= count && rank < count * 2){
      MPI_Recv(array_local, arr_size, MPI_DOUBLE, rank - count, 111, MPI_COMM_WORLD, &status[rank]);
      MPI_Get_elements(&status[rank],MPI_DOUBLE,&arr_size);
      MPI_Barrier(MPI_COMM_WORLD);
      printf("\nrank %d rec elements: %d \n",rank, arr_size);
      print_array(array_local, arr_size, rank);
    }
    count *= 2;
    //    MPI_Barrier(MPI_COMM_WORLD);
  }

  printf("\n proc: %d, arr_size: %d \n", rank, arr_size);


  //  MPI_Barrier(MPI_COMM_WORLD);
  printf("\nAfter barrier, rank = %d arr_size = %d\n", rank, arr_size);
  quick_sort(array_local, arr_size, rank);
  //  MPI_Barrier(MPI_COMM_WORLD);
  print_array(array_local, arr_size, rank);


  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Gather(&arr_size, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //  MPI_Barrier(MPI_COMM_WORLD);
  // recv_displ[0] = 0;
  if(rank ==0){
    int index = 0; recv_displ[0] = index;
    for(i=1;i<size;i++){
      //   recv_displ[i]=recv_displ[i-1]+recv_counts[i-1];
      index = index + recv_counts[i-1];
      recv_displ[i] = index;
    }


    //print recv_counts and recv_displ to check if correct

    printf("\n recv_counts: \n");
    for(i=0; i<size ;i++){
      printf(" %d ", recv_counts[i]);
    }
    printf("\n recv_displ: \n");
    for(i=0; i<size ;i++){
      printf(" %d ", recv_displ[i]);
    }
  }


  MPI_Barrier(MPI_COMM_WORLD);
  printf("\n Before gather rank: %d, arr_size = %d\n",rank, arr_size);

  // if(size>1){
  // MPI_Barrier(MPI_COMM_WORLD);
  /* MPI_Type_vector(1, arr_size, arr_size, MPI_DOUBLE, &arr); */
  /* MPI_Type_commit(&arr); */
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Gatherv(&array_local[0],arr_size, MPI_DOUBLE, array,recv_counts, recv_displ, MPI_DOUBLE,0, MPI_COMM_WORLD);
  printf("\ntjoho proc %d är efter gatherv!", rank);
  //   MPI_Gatherv(array_local,1, arr, array,recv_counts, recv_displ, MPI_DOUBLE,0, MPI_COMM_WORLD);
  //  MPI_Barrie
  //  }
   MPI_Barrier(MPI_COMM_WORLD);
 
  if(rank == 0){ 
    print_array(array, 10, rank);
    free(array);
  }
  
  free(array_local);
  //  free(array);
  /* free(recv_displ);
     free(recv_counts);*/
  MPI_Finalize();
  return 0;

}

void quick_sort(double *A, int A_size, int R){
  int i, j;
  if(A_size<2) return;
  int pivot = A[A_size/2];
  printf("\nproc %d in quick_sort", R);
  for (i = 0, j = A_size - 1; ; i++, j--)
    {printf("\n proc %d in quicksort for-loop, i=%d, j=%d\n", R, i, j);
      while (A[i] < pivot){
	printf("\n proc %d in i-while\n", R);
	i++;}
      while (A[j] > pivot){
	printf("\n proc %d in j-while\n", R);
	j--;}
      if (i >= j) break;
 
      swap(&A[i], &A[j]);
    }
  quick_sort(A,i,R);
  quick_sort(A+i, A_size-i, R);
}

void swap(double *a, double *b)
{
  int temp;
  temp = *a;
  *a = *b;
  *b = temp;
}


void print_array(double *A, int A_size, int R){
  printf("\nrank: %d Array: ", R); 
  int i;
  for(i=0; i<A_size ;i++){
    printf(" %g ", A[i]);
  }
  printf("\n ");
}


