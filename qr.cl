#pragma OPENCL EXTENSION cl_amd_printf : enable

__kernel void qr(__local float *u_vec, __global float *a_mat, 
      __global float *q_mat, __global float *p_mat, 
      __global float *prod_mat, int N, int M) {

//printf("%d\n",get_num_groups(0));
//printf("%d\n",get_group_id(0));
   local float u_length_squared, dot;
   float prod ;
	int t ;
	int h = 2;
int j;
	local float  vec_length;
	//local int grp_id;
	
	//printf("size = %d", sizeof(u_vec));
//printf("size = %d", sizeof(a_mat));
   int id = get_global_id(0);
int id_1 = get_local_id(0);
  // int num_cols = get_global_size(0);
	int num_cols = N;


   /* Load first column into local memory as u vector */
//if((id+1)%(id_1+1)==0){
for(t = 0; t < h; t++){
   u_vec[id_1 + t*N] = a_mat[id*num_cols + (get_group_id(0)+t)*(M*N/4)];
//printf("kk= %d\n",id*num_cols);
	//printf("a[%d] = %f\n\n", id*num_cols + t*(M*N), a_mat[id*num_cols + t*(M*N/2)]);
//printf("u_vec_l[%d][%d]a[%d] = %f\n", id_1 + t*N, get_group_id(0), id*num_cols + (get_group_id(0)+t)*(M*N/4), u_vec[id_1 + t*N]);
}
	//}
   barrier(CLK_LOCAL_MEM_FENCE);

   /* Find length of first A column and u vector */
   if(id_1 ==  0) {
	//printf("id = %d\n",id);
	vec_length = 0.0f;
      for(int i=1; i<2*num_cols; i++) {
	//printf("u_vec[%d] = %f\n", id, u_vec[id_1]);
         vec_length += u_vec[i] * u_vec[i];
      }
      u_length_squared = vec_length;
      vec_length = sqrt(vec_length + u_vec[0] * u_vec[0]);
	printf("len = %f\n", vec_length);
	//grp_id = get_group_id
	a_mat[get_group_id(0)*(N*M/2)] = vec_length;
      u_vec[0] -= vec_length;
      u_length_squared += u_vec[0] * u_vec[0];
   }
   //else {
   //   a_mat[id_1*num_cols] = 0.0f;
  // 
//}
if((id*num_cols) % (N*N) != 0){
a_mat[id*num_cols] = 0.0f;}
 
//printf("a[16] = %f\n",a_mat[16]);
//if(id % (num_cols*num_cols) == 0){      
//	a_mat[id] = vec_length;
//printf("amat[%d]\n",id);
//}
   barrier(CLK_GLOBAL_MEM_FENCE);

   /* Transform further columns of A */
   for(int i=1; i<num_cols; i++) {
      dot = 0.0f;
//printf("%d\n",get_group_id(0));
      if(id_1 == 0) {
         for(j=0; j<M/2; j++) {
	//printf("a[%d]\n",j*num_cols + i + (get_group_id(0)*M*N/2));
            dot += a_mat[j*num_cols + i + (get_group_id(0)*(M*N/2))] * u_vec[j];
         }
      }
      barrier(CLK_LOCAL_MEM_FENCE);
	//printf("%d\n",id_1*num_cols + i + (get_group_id(0)*16));
      a_mat[id_1*num_cols + i + (get_group_id(0)*(M*N/2))] -= 2 * u_vec[id_1] * dot / u_length_squared;
	//printf("a[%d] = %f\n",id_1*num_cols + i + (get_group_id(0)*(M*N/2)),a_mat[id_1*num_cols + i + (get_group_id(0)*(M*N/2))]);
   }

   /* Update Q matrix */

   for(int i=0; i<M/2; i++) {
      q_mat[id_1 + i + (get_group_id(0)*(M*M/4))] = -2 * u_vec[i] * u_vec[id_1] / u_length_squared;
printf("q[%d][%d] = %f\n",id_1 + i + (get_group_id(0)*(M*M/4)),get_group_id(0),q_mat[id_1 + i + (get_group_id(0)*(M*M/4))]);
   }
   q_mat[id_1*num_cols + id_1 +(get_group_id(0)*(M*N/2))] += 1;
//printf("q[%d] = %f\n",id_1*num_cols + i + (get_group_id(0)*16),q_mat[id_1*num_cols + i + (get_group_id(0)*16)]);
   barrier(CLK_GLOBAL_MEM_FENCE); 

   /* Loop through other columns */
   for(int col = 1; col < num_cols-1; col++) {

      /* Load new column into memory */
      u_vec[id_1] = a_mat[id_1 * num_cols + col + (get_group_id(0)*(N*N))];
//printf("u_vec_l[%d] = %f\n", id_1, u_vec[id_1]);
      barrier(CLK_LOCAL_MEM_FENCE);

      /* Find length of A column and u vector */
      if(id_1 == col) {//printf("yo\n");
         vec_length = 0.0f;
         for(int i = col + 1; i < num_cols; i++) {
            vec_length += u_vec[i] * u_vec[i];
         }
         u_length_squared = vec_length;
         vec_length = sqrt(vec_length + u_vec[col] * u_vec[col]);
         u_vec[col] -= vec_length;
         u_length_squared += u_vec[col] * u_vec[col];
         a_mat[col * num_cols + (get_group_id(0)*(N*N)) + col] = vec_length;
      }
      else if(id_1 > col) {//printf("yo1\n");
         a_mat[id_1 * num_cols + (get_group_id(0)*(N*N)) + col] = 0.0f;
	//printf("a[%d] = %f\n", id_1 * num_cols + (get_group_id(0)*16) + col, a_mat[id_1 * num_cols * (get_group_id(0)*16) + col]);
      }
      barrier(CLK_GLOBAL_MEM_FENCE);

      /* Transform further columns of A */
      for(int i = col+1; i < num_cols; i++) {
         if(id_1 == 0) {
            dot = 0.0f;
            for(int j=col; j<num_cols; j++) {
               dot += a_mat[j*num_cols + i + (get_group_id(0)*(N*N))] * u_vec[j];
            }
         }
         barrier(CLK_LOCAL_MEM_FENCE);
         
         if(id_1 >= col)
            a_mat[id_1*num_cols + i + (get_group_id(0)*(N*N))] -= 2 * u_vec[id_1] * 
                  dot / u_length_squared;
         barrier(CLK_GLOBAL_MEM_FENCE);
      }

      /* Update P matrix */
      if(id_1 >= col) {
         for(int i=col; i<num_cols; i++) {
            p_mat[id_1*num_cols + i + (get_group_id(0)*(N*N))] = -2 * u_vec[i] * 
                  u_vec[id_1] / u_length_squared;
         }
         p_mat[id_1*num_cols + id_1 +(get_group_id(0)*(N*N))] += 1;
      }
      barrier(CLK_GLOBAL_MEM_FENCE); 

      /* Multiply q_mat * p_mat = prod_mat */
      for(int i=col; i<num_cols; i++) {
         prod = 0.0f;
         for(int j=col; j<num_cols; j++) {
            prod += q_mat[id_1*num_cols + j + (get_group_id(0)*(N*N))] * p_mat[j*num_cols + i + (get_group_id(0)*(N*N))];
         }     
         prod_mat[id_1*num_cols + i +(get_group_id(0)*(N*N))] = prod;  
      }
      barrier(CLK_GLOBAL_MEM_FENCE); 

      /* Place the content of prod_mat in q_mat */
      for(int i=col; i<num_cols; i++) {
         q_mat[id_1*num_cols + i +(get_group_id(0)*(N*N))] = prod_mat[id_1*num_cols + i +(get_group_id(0)*(N*N))];
      }
      barrier(CLK_GLOBAL_MEM_FENCE); 
   }
}
