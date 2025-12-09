#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include <riscv_vector.h>
#define N 16


void fill_random(uint8_t *vec, size_t length, int min, int max);
void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c);
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c);
uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){

	uint8_t X[N];
	uint8_t Y[N]; 
	uint8_t Z1[N] = {0};
	uint8_t Z2[N] = {0};

	srand((unsigned int)time(NULL));
	fill_random(X,N,0,127);
	fill_random(Y,N,0,127);

	golden_sum(X,Y,Z1);

	intrinsics_sum(X,Y,Z2);

	for(int i = 0; i < N; i++){
	printf("%u %u\n", Z1[i], Z2[i]);
	}

	printf("Golden_seconds: %f\nIntrinsics_seconds: %f\n", (float)cycles_golden/CLOCKS_PER_SEC, (float)cycles_intrinsics/CLOCKS_PER_SEC);

return 0;
}

// C coded vector sum with simple for loop
void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c){
	//asm volatile ("rdcycle %0" : "=r" (start));
	start = clock();
		for (int i = 0; i < N; i++ ){
			c[i] = a[i] + b[i]; //dumb vector sum
		}

	//asm volatile ("rdcycle %0" : "=r" (end));
	end = clock();
	cycles_golden = end-start;
}

// intrinsics coded vector sum with the vector contained in a single vector register
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c){
	//asm volatile ("rdcycle %0" : "=r" (start));	
	start = clock();
		size_t vlmax = __riscv_vsetvlmax_e8m1();		//set of vlmax given SEW=8 and LMUL=1
									//
		vuint8m1_t v0 = __riscv_vle8_v_u8m1 (a, vlmax);	//vector load for byte-unsigned, LMUL=1
		vuint8m1_t v1 = __riscv_vle8_v_u8m1 (b, vlmax);

		v0 = __riscv_vadd_vv_u8m1 (v0, v1, vlmax);	//vector add for bytes, LMUL=1

		__riscv_vse8_v_u8m1 (c, v0, vlmax);			//vector store for byte-unsigned, LMUL=1

	//asm volatile ("rdcycle %0" : "=r" (end));
	end = clock();
	cycles_intrinsics = end-start;
}


void fill_random(uint8_t *vec, size_t length, int min, int max) {

    for (int i = 0; i < length; i++) {
        vec[i] = min + rand() % (max - min + 1);
    }
}
