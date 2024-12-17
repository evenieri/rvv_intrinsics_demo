#include <stdio.h>
#include <stdint.h>

#include <riscv_th_vector.h>
#define N 16

void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c);
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c);
uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){

	uint8_t X[N] = {7,28,7,52,47,0,58,57,37,108,21,42,2,8,87,120};
	uint8_t Y[N] = {9,47,48,6,23,29,17,7,70,17,6,81,19,10,106,116};
	uint8_t Z1[N] = {0};
	uint8_t Z2[N] = {0};

	golden_sum(X,Y,Z1);

	intrinsics_sum(X,Y,Z2);

	for(int i = 0; i < N; i++){
	printf("%u %u\n", Z1[i], Z2[i]);
	}

	printf("Golden_cycles: %llu\nIntrinsics_cycles: %llu\n", cycles_golden, cycles_intrinsics);

return 0;
}

// C coded vector sum with simple for loop
void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c){
	asm volatile ("rdcycle %0" : "=r" (start));

		for (int i = 0; i < N; i++ ){
			c[i] = a[i] + b[i]; //dumb vector sum
		}

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_golden = end-start;
}

// intrinsics coded vector sum with the vector contained in a single vector register
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c){
	asm volatile ("rdcycle %0" : "=r" (start));	

		size_t vlmax = __riscv_vsetvlmax_e8m1();		//set of vlmax given SEW=8 and LMUL=1
									//
		vuint8m1_t v0 = __riscv_th_vlbu_v_u8m1 (a, vlmax);	//vector load for byte-unsigned, LMUL=1
		vuint8m1_t v1 = __riscv_th_vlbu_v_u8m1 (b, vlmax);

		v0 = __riscv_vadd_vv_u8m1 (v0, v1, vlmax);	//vector add for bytes, LMUL=1

		__riscv_th_vsb_v_u8m1 (c, v0, vlmax);			//vector store for byte-unsigned, LMUL=1

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_intrinsics = end-start;
}
