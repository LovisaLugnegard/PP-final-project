#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void swap(double*, double*);
void partition_array(double, int);
void print_array();
void quick_sort(double *A, int A_size);
void sort_incoming_arrays(double *A, int A_size, int *recv_order);
int confirm_sorted(double *A, int size);

int count = 1;
int main(int argc, char *argv[]) {

  /* Initialization */
  int i, arr_size, j, rank, size, total_arr_size;
  double *array, *array_local;
  double pivot, a, time_init, time_end;

  MPI_Datatype strided, arr;
  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */
  MPI_Status status[size];
  MPI_Request request[size];
  arr_size=10000000;
  total_arr_size = arr_size;
  array_local = malloc(arr_size*sizeof(double));

  int recv_counts[size];
  int recv_displ[size];
  double sorting[size];
  int recv_order[size];
  if(rank == 0){ 
    array = malloc(arr_size*sizeof(double)); //Initialize test array
    for(i=0;i<arr_size;i++){
      array_local[i] = rand();
    }
    time_init = MPI_Wtime(); //start timer  
  }

  MPI_Barrier(MPI_COMM_WORLD);
  while (count < size){  
    if (rank < count && rank + count < size){
      //partition array according to pivot
      pivot = array_local[1];
      for (i = 0, j = arr_size - 1; ; i++, j--){
	while (array_local[i] < pivot){
	  i++;}
	while (array_local[j] > pivot){
	  j--;
	}
	if (i >= j) break;
 
	swap(&array_local[i], &array_local[j]);
      }     
      /* Send partition to new process */
      MPI_Type_vector(1, arr_size-i, arr_size-i, MPI_DOUBLE, &strided);
      MPI_Type_commit(&strided);
      MPI_Send(&array_local[i], 1, strided, rank + count, 111, MPI_COMM_WORLD);   

      arr_size = i;
    }
    
    if (rank >= count && rank < count * 2){
      /* Receive partition */
      MPI_Recv(array_local, arr_size, MPI_DOUBLE, rank - count, 111, MPI_COMM_WORLD, &status[rank]);
      MPI_Get_elements(&status[rank],MPI_DOUBLE,&arr_size);
    }
    count *= 2;
      MPI_Barrier(MPI_COMM_WORLD);
  }


  if(arr_size>1){
    quick_sort(array_local, arr_size); //serial quick sort
  }

  MPI_Barrier(MPI_COMM_WORLD);
  /* Gather needed information from all processes in the root */
  MPI_Gather(&arr_size, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Gather(&array_local[0], 1, MPI_DOUBLE, sorting, 1, MPI_DOUBLE,0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  if(rank ==0){
    for(i=0;i<size;i++){
      recv_order[i] = i;
    }

    for(i=0;i<size;i++){
      if(recv_counts[i] == 0){
	sorting[i] = 0;
      }
    }

    sort_incoming_arrays(sorting, size, recv_order);
    recv_displ[0] = 0;
    for(i=1;i<size;i++){
      recv_displ[i]=recv_displ[i-1]+recv_counts[recv_order[i-1]];
    }

  }


  MPI_Barrier(MPI_COMM_WORLD);
  /* Gather all sorted partitions in the root */
  MPI_Type_vector(1, arr_size, arr_size, MPI_DOUBLE, &arr); 
  MPI_Type_commit(&arr); 
  MPI_Isend(array_local,1,arr,0,4, MPI_COMM_WORLD,&request[rank]);

  if(rank==0){
    for(i=0; i<size; i++){
      MPI_Recv(&array[recv_displ[i]],total_arr_size, MPI_DOUBLE,recv_order[i],4, MPI_COMM_WORLD,&status[rank]);
 
    }
  }
 
  MPI_Barrier(MPI_COMM_WORLD);
 
  if(rank == 0){
    time_end = MPI_Wtime() - time_init;
    printf("\nElapsed time: %.16f s\n", time_end); 
    if(!confirm_sorted(array,total_arr_size)){
      printf("\n Error array not sorted!\n");}
    else{
      printf("\n Array sorted!\n");}
    free(array);

  }
  
  free(array_local);

  MPI_Finalize();
  return 0;

}

/*serial quick sort */
void quick_sort(double *A, int A_size){
  int i, j;
  if(A_size<2) return;
  int pivot = A[A_size/2];
  for(i = 0, j = A_size - 1; ; i++, j--){
    while (A[i] < pivot){	i++;}
    while (A[j] > pivot){
      j--;}
    if (i >= j) break;
    swap(&A[i], &A[j]);
  }
  quick_sort(A,i);
  quick_sort(A+i, A_size-i);
}

void swap(double *a, double *b)
{
  int temp;
  temp = *a;
  *a = *b;
  *b = temp;
}

void swap_int(int *a, int *b)
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

void sort_incoming_arrays(double *A, int A_size, int *recv_order){
  int i, j;
  if(A_size<2) return;
  int pivot = A[A_size/2];
  for (i = 0, j = A_size - 1; ; i++, j--)
    {
      while (A[i] < pivot){	
	i++;}
      while (A[j] > pivot){
	j--;}
      if (i >= j) break;

      swap(&A[i], &A[j]);
      swap_int(&recv_order[i], &recv_order[j]);
    }
  sort_incoming_arrays(A,i, recv_order);
  sort_incoming_arrays(A+i, A_size-i, recv_order+i);
}

int confirm_sorted(double *A, int size){
  int i;
  for(i=0; i<size-1; i++){
    if(A[i] > A[i+1]){
      return 0;
    }
  }
  return 1;
}
