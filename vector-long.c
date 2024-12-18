#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <riscv_th_vector.h>
#define N 10000
#define M 128

void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c);
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c);
void check_sum (uint8_t * a, uint8_t * b);

uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){
	

	uint8_t X[N];
	uint8_t Y[N];
	srand(0xdeafbeef);

	for(int i = 0; i < N; i++){
		X[i] = rand() % M;
		Y[i] = rand() % M;
	}

	uint8_t Z1[N];
	uint8_t Z2[N];

	golden_sum(X,Y,Z1);

	intrinsics_sum(X,Y,Z2);

	check_sum(Z1, Z2);

	printf("Golden_cycles: %lu\nIntrinsics_cycles: %lu\n", cycles_golden, cycles_intrinsics);

return 0;
}


void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c){
	asm volatile ("rdcycle %0" : "=r" (start));

		for (int i = 0; i < N; i++ ){
			c[i] = a[i] + b[i]; //dumb vector sum
		}

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_golden = end-start;
}

void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c){
	asm volatile ("rdcycle %0" : "=r" (start));

	size_t vlmax = __riscv_vsetvlmax_e8m8();				//vsetvlmax given 8 bit elements and group of 8 registers

		for (int i = 0; i < N; i += vlmax ){
		vlmax = __riscv_vsetvl_e8m8(N-i);
		vuint8m8_t v0 = __riscv_th_vlbu_v_u8m8 (&a[i], vlmax);		//vector load for byte-unsigned, LMUL=8
		vuint8m8_t v1 = __riscv_th_vlbu_v_u8m8 (&b[i], vlmax);

		v0 = __riscv_vadd_vv_u8m8 (v0, v1, vlmax);			//vector add for bytes, LMUL=8

		__riscv_th_vsb_v_u8m8 (&c[i], v0, vlmax);			//vector store for byte-unsigned, LMUL=8
		}

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_intrinsics = end-start;
}

void check_sum (uint8_t * a, uint8_t * b){
	int check = 0;
	for(int i = 0; i < N; i++){
		check += (a[i] - b[i]);	
	}

	if(!check) printf("OK!\n");
       		else printf("Not OK :/\n");

}
