#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include <riscv_vector.h>
#define N 2048


void fill_random(uint8_t *vec, size_t length, int min, int max);
void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c);
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c);
void result_cmp(uint8_t *a, uint8_t *b, size_t length);
uint64_t start, end, cycles_golden, cycles_intrinsics;

int main(){

	uint8_t X[N];
	uint8_t Y[N]; 
	uint8_t Z1[N] = {0};
	uint8_t Z2[N] = {0};

	srand((unsigned int)time(NULL));
	fill_random(X, N, 0, 127);
	fill_random(Y, N, 0, 127);

	golden_sum(X, Y, Z1);

	intrinsics_sum(X, Y, Z2);

	result_cmp(Z1, Z2, N);

	printf("Golden_seconds: %f\nIntrinsics_seconds: %f\n", (float)cycles_golden/CLOCKS_PER_SEC, (float)cycles_intrinsics/CLOCKS_PER_SEC);

return 0;
}

// C coded vector sum with simple for loop
void golden_sum(uint8_t * a, uint8_t * b, uint8_t * c){
	start = clock();
		for (int i = 0; i < N; i++ ){
			c[i] = a[i] + b[i]; //dumb vector sum
		}

	end = clock();
	cycles_golden = end-start;
}

// intrinsics coded vector sum with the vector contained in a single vector register
void intrinsics_sum(uint8_t * a, uint8_t *b, uint8_t * c){
	start = clock();
	
	size_t i=0;
	size_t vlmax = __riscv_vsetvl_e8m1(N-i);
	
	for(i; i < N ; i += vlmax){
		vlmax = __riscv_vsetvl_e8m1(N-i);		//set of vlmax given SEW=8 and LMUL=1
									//
		vuint8m1_t v0 = __riscv_vle8_v_u8m1 (&a[i], vlmax);		//vector load for byte-unsigned, LMUL=1
		vuint8m1_t v1 = __riscv_vle8_v_u8m1 (&b[i], vlmax);

		v0 = __riscv_vadd_vv_u8m1 (v0, v1, vlmax);		//vector add for bytes, LMUL=1

		__riscv_vse8_v_u8m1 (&c[i], v0, vlmax);			//vector store for byte-unsigned, LMUL=1
	}

	end = clock();
	cycles_intrinsics = end-start;
}


void fill_random(uint8_t *vec, size_t length, int min, int max) {

    for (size_t i = 0; i < length; i++) {
        vec[i] = min + rand() % (max - min + 1);
    }
}

void result_cmp(uint8_t *a, uint8_t *b, size_t length){
	
	size_t check = 0;
	for(size_t i = 0; i < length; i++){
		check += a[i] - b[i];
	}
	if (check){
		printf("Error\n");
	} else printf("Good!\n");

}
