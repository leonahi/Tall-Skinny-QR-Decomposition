#ifndef GEN_1_COL_H  /* Include guard */
#define GEN_1_COL_H

int * gen_col_1(int M_1, int N_1)
{	
	int n = 0;
        int m = 0;
        int k = 1;
        int i;
	int *x = (int*)malloc(M_1*N_1*(sizeof(int)));
	for (i = 0; i < M_1*N_1; i++ ){
		x[i] = m;
		m = m + 1;
		if (m == k*N_1){
			m = m - N_1;
			n = n + 1;
		}
		if (n == N_1){
			n = 0;
			k = k + 1;
			m = m + N_1;
		}
	}
		for (i = 0; i < M_1*N_1; i++){
		x[i]=x[i]+1;}
	return x;
}

void * gen_row_1(int M_1, int N_1)
{	
	int i;
	int *x = (int*)malloc((M_1+1)*(sizeof(int)));
	for(i = 0; i < M_1; i++){
	x[i]= i*N_1 +1;
	//x[i]= i*N_1 ;
	}

	x[M_1]=(M_1*N_1 + 1);
	//x[M_1]=(M_1*N_1 );
	return x;
}


cl_device_id create_device() {

  cl_platform_id platform;
  cl_device_id dev;
  int err;

  /* Identify a platform */
  err = clGetPlatformIDs(1, &platform, NULL);
  if(err < 0) {
     perror("Couldn't identify a platform");
     exit(1);
  }

  /* Access a device */
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
  if(err == CL_DEVICE_NOT_FOUND) {
     printf("GPU not found switch to CPU\n");
     err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
  }
  if(err < 0) {
     perror("Couldn't access any devices");
     exit(1);
  }

  return dev;
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

  cl_program program;
  FILE *program_handle;
  char *program_buffer, *program_log;
  size_t program_size, log_size;
  int err;

  /* Read program file and place content into buffer */
  program_handle = fopen(filename, "r");
  if(program_handle == NULL) {
     perror("Couldn't find the program file");
     exit(1);
  }
  fseek(program_handle, 0, SEEK_END);
  program_size = ftell(program_handle);
  rewind(program_handle);
  program_buffer = (char*)malloc(program_size + 1);
  program_buffer[program_size] = '\0';
  fread(program_buffer, sizeof(char), program_size, program_handle);
  fclose(program_handle);

  /* Create program from file */
  program = clCreateProgramWithSource(ctx, 1,
     (const char**)&program_buffer, &program_size, &err);
  if(err < 0) {
     perror("Couldn't create the program");
     exit(1);
  }
  free(program_buffer);

  /* Build program */
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if(err < 0) {

     /* Find size of log and print to std output */
     clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
           0, NULL, &log_size);
     program_log = (char*) malloc(log_size + 1);
     program_log[log_size] = '\0';
     clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
           log_size + 1, program_log, NULL);
     printf("%s\n", program_log);
     free(program_log);
     exit(1);
  }
  return program;
}

float* Init_m(int M_1, int N_1){
int i;
float *a = (float*)malloc(M_1*N_1*sizeof(float));
srand((unsigned int)time(0));
  for(i=0; i<M_1*N_1; i++) {
        a[i] = (float)rand()/RAND_MAX;
   }
return a;}

float* Init_m0(int M_1, int N_1){
int i;
float *a = (float*)malloc(M_1*N_1*sizeof(float));
return a;}

void Print_m(float* a, int M_1, int N_1){
int i,j;
for(i=0; i<M_1; i++) {
       for(j=0; j<N_1; j++) {
          printf("%f ",a[i*N_1 + j]);
       }
       printf(";\n");
    }}


#endif
