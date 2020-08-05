kernel void reduction_scalar(global float* data, local float* partial_sums, global float* output)
    {
    int lid = get_local_id(0);
    int group_size = get_local_size(0);
    // Copy the input data to local memory, and wait for all the work-items
    // in the work-group to complete this operation
    partial_sums[lid] = data[get_global_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    for(int i = group_size/2; i>0; i >>= 1)
        {
        if(lid < i) partial_sums[lid] += partial_sums[lid + i];
        // Wait for all the work-items in the work-group to complete this iteration
        barrier(CLK_LOCAL_MEM_FENCE);
        }

    // At this point,  partial_sums[0] contains the partial sum for this work-group.
    // Copy it to the output buffer at the index for this work-group:
    if(lid == 0) output[get_group_id(0)] = partial_sums[0];
    }


kernel void reduction_vector(global float4* data, local float4* partial_sums, global float* output)
    {
    int lid = get_local_id(0);
    int group_size = get_local_size(0);
    partial_sums[lid] = data[get_global_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    for(int i = group_size/2; i>0; i >>= 1)
        {
        if(lid < i) partial_sums[lid] += partial_sums[lid + i];
        barrier(CLK_LOCAL_MEM_FENCE);
        }

    if(lid == 0) output[get_group_id(0)] = dot(partial_sums[0], (float4)(1.0f));
    }

