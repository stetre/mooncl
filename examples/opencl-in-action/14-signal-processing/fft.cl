#define mask_left fft_index
#define mask_right stage
#define shift_pos N2
#define angle size
#define start br.s0
#define cosine x3.s0
#define sine x3.s1
#define wk x2

kernel void fft_init(global float2* in_data, global float2* out_data,
    local float2* l_data, uint points_per_group, uint size, int dir)
    {
    uint4 br, index;
    uint points_per_item, g_addr, l_addr, i, fft_index, stage, N2;
    float2 x1, x2, x3, x4, sum12, diff12, sum34, diff34;

    points_per_item = points_per_group/get_local_size(0);
    l_addr = get_local_id(0) * points_per_item;
    g_addr = get_group_id(0) * points_per_group + l_addr;

    /* Load data from bit-reversed addresses and perform 4-point FFTs */
    for(i=0; i<points_per_item; i+=4)
        {
        index = (uint4)(g_addr, g_addr+1, g_addr+2, g_addr+3);
        mask_left = size/2;
        mask_right = 1;
        shift_pos = (uint)log2((float)size) - 1;
        br = (index << shift_pos) & mask_left;
        br |= (index >> shift_pos) & mask_right;
      
        /* Bit-reverse addresses */
        while(shift_pos > 1)
            {
            shift_pos -= 2;
            mask_left >>= 1;
            mask_right <<= 1;
            br |= (index << shift_pos) & mask_left;
            br |= (index >> shift_pos) & mask_right;
            }

        /* Load global data */
        x1 = in_data[br.s0];
        x2 = in_data[br.s1];
        x3 = in_data[br.s2];
        x4 = in_data[br.s3];

        sum12 = x1 + x2;
        diff12 = x1 - x2;
        sum34 = x3 + x4;
        diff34 = (float2)(x3.s1 - x4.s1, x4.s0 - x3.s0) * dir;
        l_data[l_addr] = sum12 + sum34;
        l_data[l_addr+1] = diff12 + diff34;
        l_data[l_addr+2] = sum12 - sum34;
        l_data[l_addr+3] = diff12 - diff34; 
        l_addr += 4;
        g_addr += 4;
        }

    /* Perform initial stages of the FFT - each of length N2*2 */
    for(N2 = 4; N2 < points_per_item; N2 <<= 1)
        {
        l_addr = get_local_id(0) * points_per_item;
        for(fft_index = 0; fft_index < points_per_item; fft_index += 2*N2)
            {
            x1 = l_data[l_addr];
            l_data[l_addr] += l_data[l_addr + N2];
            l_data[l_addr + N2] = x1 - l_data[l_addr + N2];
            for(i=1; i<N2; i++)
                {
                cosine = cos(M_PI_F*i/N2);
                sine = dir * sin(M_PI_F*i/N2);
                wk = (float2)(l_data[l_addr+N2+i].s0*cosine + l_data[l_addr+N2+i].s1*sine, 
                          l_data[l_addr+N2+i].s1*cosine - l_data[l_addr+N2+i].s0*sine);
                l_data[l_addr+N2+i] = l_data[l_addr+i] - wk;
                l_data[l_addr+i] += wk;
                }
            l_addr += 2*N2;
            }
        }
    barrier(CLK_LOCAL_MEM_FENCE);

    /* Perform FFT with other items in group - each of length N2*2 */
    stage = 2;
    for(N2 = points_per_item; N2 < points_per_group; N2 <<= 1)
        {
        start = (get_local_id(0) + (get_local_id(0)/stage)*stage) * (points_per_item/2);
        angle = start % (N2*2);
        for(i=start; i<start + points_per_item/2; i++)
            {
            cosine = cos(M_PI_F*angle/N2);
            sine = dir * sin(M_PI_F*angle/N2);
            wk = (float2)(l_data[N2+i].s0*cosine + l_data[N2+i].s1*sine, 
                       l_data[N2+i].s1*cosine - l_data[N2+i].s0*sine);
            l_data[N2+i] = l_data[i] - wk;
            l_data[i] += wk;
            angle++;
            }
        stage <<= 1;
        barrier(CLK_LOCAL_MEM_FENCE);
        }

    /* Store results in global memory */
    l_addr = get_local_id(0) * points_per_item;
    g_addr = get_group_id(0) * points_per_group + l_addr;
    for(i=0; i<points_per_item; i+=4)
        {
        out_data[g_addr] = l_data[l_addr];
        out_data[g_addr+1] = l_data[l_addr+1];
        out_data[g_addr+2] = l_data[l_addr+2];
        out_data[g_addr+3] = l_data[l_addr+3];    
        g_addr += 4;
        l_addr += 4;
        }
    }

kernel void fft_stage(global float2* g_data, uint stage, uint points_per_group, int dir)
    {
    uint points_per_item, addr, N, ang, i;
    float c, s;
    float2 input1, input2, w;

    points_per_item = points_per_group/get_local_size(0);
    addr = (get_group_id(0) + (get_group_id(0)/stage)*stage) * (points_per_group/2) +
            get_local_id(0) * (points_per_item/2);
    N = points_per_group*(stage/2);
    ang = addr % (N*2);

    for(i=addr; i<addr + points_per_item/2; i++)
        {
        c = cos(M_PI_F*ang/N);
        s = dir * sin(M_PI_F*ang/N);
        input1 = g_data[i];
        input2 = g_data[i+N];
        w = (float2)(input2.s0*c + input2.s1*s, input2.s1*c - input2.s0*s);
        g_data[i] = input1 + w;
        g_data[i+N] = input1 - w;
        ang++;
        }
    }

kernel void fft_scale(global float2* g_data, uint points_per_group, uint scale)
    {
    uint points_per_item, addr, i;
    points_per_item = points_per_group/get_local_size(0);
    addr = get_group_id(0) * points_per_group + get_local_id(0) * points_per_item;
    for(i=addr; i<addr + points_per_item; i++)
      g_data[i] /= scale;
    }
