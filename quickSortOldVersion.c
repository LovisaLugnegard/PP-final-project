#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//void quick_sort()
void swap();

  int i, n;
  double *array;
  double pivot;

//behöver allokera minne i varje process
main(){
  n=10;
  array = malloc(n*sizeof(double));

  //Initialize test array
  for(i=0;i<n;i++){
    array[i] =2* i*1.0;
  }
  pivot = array[5];
  printf(" pivot = %g \n", pivot);

  for(i=0;i<n;i++){
    printf(" %g ", array[i]);
  }

  //partition array according to pivot

  double open_card = array[0];
  for(i=0; i<n; i++){
    if(array[i]<pivot){
      //swap with open_card, use pointers
      swap(array[i], open_card);
    }
    if(array[i]>pivot){
      open_card = array[i]; 
    }
  }

 for(i=0;i<n;i++){
    printf("Array after swapping: %g ", array[i]);
  }



  /* if(rank == 0){ */
  /* //i process 0: dela upp listan enligt pivotelement */
  /* pivot = array[0]; //first element as pivot */
  
  /* for(i=0; i<n; i++){} */
  /* 	if(array[i]<pivot){ */
  /* 		//Send element to process 1 */
  /* 	} */
  /* 	else{ */
  /* 	//put element in new array in process 0 */
  /* 	} */
  /* } */
  
  //  MPI_Finalize();
  return 0;

}

//quick_sort()

void swap(double *a,double *b)
   {
   double temp = *a;
   *a = *b;
   *b = temp;
   }
