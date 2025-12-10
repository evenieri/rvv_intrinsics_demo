#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <riscv_vector.h>
#define M 256
#define N 128

void golden_gemv(float * m, float * v, float * r, size_t rows, size_t cols);
void intrinsics_gemv(float * m, float * v, float * r, size_t rows, size_t cols);
void random_float_vector(float *vec, size_t n);
void check_gemv (float * v1, float * v2, size_t lenght);

clock_t start, end, cycles_golden, cycles_intrinsics;

int main(){
	srand((unsigned int)time(NULL));
	float * mat  = malloc(M * N * sizeof(float));
	float * vec  = malloc(N * sizeof(float));
	float * res1 = malloc(M * sizeof(float));
	float * res2 = malloc(M * sizeof(float));


	random_float_vector(mat, M*N);
	random_float_vector(vec, N);

	golden_gemv(mat, vec, res1, M, N);
	intrinsics_gemv(mat, vec, res2, M, N);

	check_gemv(res1, res2, N);

	printf("Golden_seconds: %f\nIntrinsics_seconds: %f\n", (float)cycles_golden/CLOCKS_PER_SEC, (float)cycles_intrinsics/CLOCKS_PER_SEC);

return 0;
}


void golden_gemv(float * m, float * v, float * r, size_t rows, size_t cols){
	start = clock();
		
	for (size_t i = 0; i < rows; i++){
		r[i] = 0;
		for(size_t j = 0; j < cols; j++){
			r[i] += m[i * cols + j] * v [j];
		}
	}

	end = clock();
	cycles_golden = end-start; 
}

void intrinsics_gemv(float * m, float * v, float * r, size_t rows, size_t cols){
	start = clock(); //do not touch, performance monitoring
	vfloat32m1_t v_zeros;
	vfloat32m1_t v_sum;

	for(size_t i = 0; i < rows; i++){
		size_t vlmax = __riscv_vsetvlmax_e32m8();
		vfloat32m8_t v_acc = __riscv_vfmv_v_f_f32m8(0.0,vlmax);
		for(size_t j = 0; j < cols;){
			size_t vl = __riscv_vsetvl_e32m8(cols - j);
			vfloat32m8_t v_mat_row = __riscv_vle32_v_f32m8(&m[i * cols + j], vl);
			vfloat32m8_t v_vec_col = __riscv_vle32_v_f32m8(&v[j], vl);
			v_acc = __riscv_vfmacc_vv_f32m8_tu(v_acc, v_mat_row, v_vec_col, vl);
			j += vl;
		}

		size_t vl_red = __riscv_vsetvlmax_e32m1();
       		v_zeros = __riscv_vfmv_v_f_f32m1(0.0, vl_red);	
		v_sum = __riscv_vfredusum_vs_f32m8_f32m1(v_acc, v_zeros, vlmax); 
		r[i] = __riscv_vfmv_f_s_f32m1_f32(v_sum);
	}

	end = clock(); //do not touch, performance monitoring
	cycles_intrinsics = end-start; //do not touch
}
void check_gemv (float * v1, float * v2, size_t length){
	int check = 0;
       	size_t i = 0;

	for(i; i < length; i++){
		if(v1[i] - v2[i] > 1e-3) check = 1;
		printf("%f, %f\n", v1[i], v2[i]);
	}

	if(check) printf("Error :/\n");
       		else printf("Good!\n");

}

void random_float_vector(float *vec, size_t n) {
    for (size_t i = 0; i < n; i++) {
        vec[i] = (float)rand() / (float)RAND_MAX;  // 0.0 to 1.0
    }
}

