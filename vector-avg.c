#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <riscv_th_vector.h>
#define N 4096
#define M 128

void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c);
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c);
void check_sum (uint8_t * a, uint8_t * b);

uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){
	

	uint8_t X[N];
	srand(0xdeafbeef);

	for(int i = 0; i < N; i++){
		X[i] = rand() % M;
	}

	uint32_t Z1;
	uint32_t Z2;

	golden_avg(X,Z1);

	intrinsics_avg(X,Y,Z2);

	check_avg(Z1, Z2);

	printf("Golden_cycles: %llu\nIntrinsics_cycles: %llu\n", cycles_golden, cycles_intrinsics);

return 0;
}


void golden_avg(uint8_t * a, uint32_t * c){
	asm volatile ("rdcycle %0" : "=r" (start)); 

		for (int i = 0; i < N; i++ ){
			c += (uint32_t)a[i]; //acc
		}

		c = c/N;

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_golden = end-start; 
}

void intrinsics_avg(uint8_t * a, uint32_t * c){
	asm volatile ("rdcycle %0" : "=r" (start)); //do not touch

	size_t vlmax = __riscv_vsetvlmax_e8m8();				//vsetvlmax given 8 bit elements and group of 8 registers
										
		vuint8m8_t v0 = __riscv_th_vlbu_v_u8m8 (&a[0], vlmax);  
		for (int i = 1; i < N; i += 128 ){
		vuint8m8_t v1 = __riscv_th_vlbu_v_u8m8 (&a[i], vlmax);		//vector load for byte-unsigned, LMUL=8
		v0 = __riscv_vadd_vv_u8m8 (v0, v1, vlmax);			//vector add for bytes, LMUL=8

		}


	asm volatile ("rdcycle %0" : "=r" (end)); //do not touch
	cycles_intrinsics = end-start; //do not touch
}

void check_avg (uint32_t a, uint32_t b){
	int check = 0;
	check = a-b;

	if(!check) printf("OK!\n");
       		else printf("Not OK :/\n");

}:
