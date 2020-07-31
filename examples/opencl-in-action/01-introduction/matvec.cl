kernel void matvec_mult(global float4* matrix, global float4* vector, global float* result)
	{
   	int i = get_global_id(0);
   	result[i] = dot(matrix[i], vector[0]);
	}
