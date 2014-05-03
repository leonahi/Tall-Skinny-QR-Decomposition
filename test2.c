#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "qr.cl"
#define KERNEL_FUNC "qr"


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <CL/cl.h>


#include "mkl_types.h"
#include "mkl_spblas.h"
#include "cl_mkl.h"

/* Find a GPU or CPU associated with the first available platform */


/* Create program from a file and compile it */

int main(int argc, char *argv[]) {
int M = atoi(argv[1]);
int N = atoi(argv[2]);
int K = N;

  /* Host/device d	ata structures */
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  size_t global_size, local_size;
  cl_int err, i, j, k, check;

  /* Data and buffers */
  float *a_mat, *q_mat, *r_mat, *Q_mat;

 
	a_mat = Init_m(M,N);
	q_mat = Init_m(M,N);
	Q_mat = Init_m0(M,M);
	r_mat = Init_m(M,N);


  cl_mem a_buffer, q_buffer, p_buffer, prod_buffer;

  /* Create a device and context */
  device = create_device();
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  if(err < 0) {
     perror("Couldn't create a context");
     exit(1);
  }

  /* Build the program */
  program = build_program(context, device, PROGRAM_FILE);

  /* Create a kernel */
  kernel = clCreateKernel(program, KERNEL_FUNC, &err);
  if(err < 0) {
     perror("Couldn't create a kernel");
     exit(1);
  };


  /* Create buffer */
  a_buffer = clCreateBuffer(context,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        (M*N)*sizeof(float), a_mat, &err);
  if(err < 0) {
     perror("Couldn't create a buffer");
     exit(1);
  };
  q_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		  (M*M)*sizeof(float), NULL, NULL);
  p_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		  (M*N)*sizeof(float), NULL, NULL);
  prod_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		  (M*N)*sizeof(float), NULL, NULL);

  /* Create kernel arguments */
  err = clSetKernelArg(kernel, 0, (N * sizeof(float)), NULL);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &a_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &q_buffer);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &p_buffer);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &prod_buffer);
  err |= clSetKernelArg(kernel, 5, sizeof(K), &K);
  if(err < 0) {
     printf("Couldn't set a kernel argument");
     exit(1);
  };

  /* Create a command queue */
  queue = clCreateCommandQueue(context, device, 0, &err);
  if(err < 0) {
     perror("Couldn't create a command queue");
     exit(1);
  };

  /* Enqueue kernel */
  global_size = M;
  local_size = N;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size,
        &local_size, 0, NULL, NULL);
  if(err < 0) {
     perror("Couldn't enqueue the kernel");
     exit(1);
  }

  /* Read the results */
  err = clEnqueueReadBuffer(queue, q_buffer, CL_TRUE, 0,
		  (M*M)*sizeof(float), q_mat, 0, NULL, NULL);
  err |= clEnqueueReadBuffer(queue, a_buffer, CL_TRUE, 0,
		  (M*N)*sizeof(float), r_mat, 0, NULL, NULL);
  if(err < 0) {
     perror("Couldn't read the buffers");
     exit(1);
  }

Print_m(a_mat, M, N);
Print_m(q_mat, M, N);
Print_m(r_mat, M, N);

char transa = 'n';



MKL_INT *columns = gen_col_1(M,N);
MKL_INT *rowIndex1 = gen_row_1(M,N);

int m = M;
int n = N;

mkl_scsrmultd (&transa,&m, &m, &m, q_mat, columns, rowIndex1, q_mat, columns, rowIndex1, Q_mat, &m);

Print_m(Q_mat, M, M);

  /* Deallocate resources */
  clReleaseMemObject(a_buffer);
  clReleaseMemObject(q_buffer);
  clReleaseMemObject(p_buffer);
  clReleaseMemObject(prod_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
  free(a_mat);
free(q_mat);
free(Q_mat);
free(r_mat);


  return 0;
}
