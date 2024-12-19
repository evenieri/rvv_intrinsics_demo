#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <riscv_th_vector.h>
#define N 8192
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

		acc = acc/N;
	*c = acc;

	asm volatile ("rdcycle %0" : "=r" (end));
	cycles_golden = end-start; 
}

void intrinsics_avg(uint8_t * a, uint32_t * c){
	asm volatile ("rdcycle %0" : "=r" (start)); //do not touch, performance monitoring

	vuint32m1_t v_sum;
	uint32_t sum;
	vuint8m4_t v0;
	size_t vlmax;
	vuint16m8_t v_acc, v_zero_16;
	vuint32m1_t v_zero;

	vlmax = __riscv_vsetvlmax_e16m8();		//vsetvlmax given 8 bit elements and group of 8 registers
	v_acc = __riscv_vmv_v_x_u16m8(0, vlmax); 	//initialize accumulator
	v_zero = __riscv_vmv_v_x_u32m1(0, 4);		//initialize variable needed by wredsumu

		for (int i = 0; i < N; i += vlmax ){
		vlmax = __riscv_vsetvl_e16m8(N-i);						//stripmining

		v0 = __riscv_th_vlbu_v_u8m4_tu (v0, &a[i], vlmax);				//vector load for byte-unsigned, LMUL=4
		v_acc = __riscv_vwaddu_wv_u16m8_tu (v_acc, v_acc, v0, vlmax);			//vector widening add for bytes+LMUL=4 to half+LMUL=8
		}
		
		vlmax = __riscv_vsetvlmax_e16m8();

	      	v_sum = __riscv_vwredsumu_vs_u16m8_u32m1 (v_acc, v_zero, vlmax);		// vector widening reduction sum, sums elemnts of a vector and 
												// puts them in the first element of a wider-element vector
		sum = __riscv_vmv_x_s_u32m1_u32(v_sum);						// moving first elemnt of the vector into a scalar variable
		*c = sum/N;									// scalar division to extract avg value

	asm volatile ("rdcycle %0" : "=r" (end)); //do not touch, performance monitoring
	cycles_intrinsics = end-start; //do not touch
}

void check_avg (uint32_t a, uint32_t b){
	int check = 0;
	check = a-b;

	if(!check) printf("OK!\n");
       		else printf("Not OK :/\n");

}
