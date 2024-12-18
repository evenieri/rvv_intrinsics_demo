#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <riscv_th_vector.h>
#define N 4096
#define M 128

void golden_avg(uint8_t * a, uint32_t * c);
void intrinsics_avg(uint8_t * a, uint32_t * c);
void check_avg (uint32_t a, uint32_t b);

uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){
	

	uint8_t X[N];
	srand(0xdeafbeef);
	for(int i = 0; i < N; i++){
		X[i] = rand() % M;
	} //random vector 

	uint32_t Z1;
	uint32_t Z2;

	golden_avg(X,&Z1);

	intrinsics_avg(X,&Z2);

	check_avg(Z1, Z2);

	printf("Golden_cycles: %llu\nIntrinsics_cycles: %llu\n", cycles_golden, cycles_intrinsics);

return 0;
}


void golden_avg(uint8_t * a, uint32_t * c){
	asm volatile ("rdcycle %0" : "=r" (start));

	uint32_t acc = 0;

		for (int i = 0; i < N; i++ ){
			acc += (uint32_t)a[i];
		}

printf("%u\n", acc);
		acc = acc/N;
	*c = acc;

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_golden = end-start; 
}

void intrinsics_avg(uint8_t * a, uint32_t * c){
	asm volatile ("rdcycle %0" : "=r" (start)); //do not touch, performance monitoring
//implement your average function with vector intrinsics






	asm volatile ("rdcycle %0" : "=r" (end)); //do not touch, performance monitoring
	cycles_intrinsics = end-start; //do not touch
}

void check_avg (uint32_t a, uint32_t b){
	int check = 0;
	check = a-b;

	if(!check) printf("OK!\n");
       		else printf("Not OK :/\n");

}
