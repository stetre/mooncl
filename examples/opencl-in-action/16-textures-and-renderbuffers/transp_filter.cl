constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | 
      CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; 

kernel void texture_filter(read_only image2d_t src_image, global uchar* dst_buffer)
    {
    int x = get_global_id(0);
    int y = get_global_id(1);

    /* Compute two-dimensional dot product */
    int pixel = read_imageui(src_image, sampler, (int2)(x,   y)).s0;

    /* Set output pixel */
    dst_buffer[y*get_global_size(0) + x] = (uchar)clamp(pixel, 0, 255);
    }
