#define U 8
#define add1 4
#define add2 2

int FullyConnected(signed char* input,
					signed char* weight,
					int input_offset,
					int weight_offset,
					int accum_depth)
{
	#pragma HLS INTERFACE s_axilite port=input_offset bundle=CRTL_BUS
	#pragma HLS INTERFACE s_axilite port=weight_offset bundle=CRTL_BUS
	#pragma HLS INTERFACE s_axilite port=accum_depth bundle=CRTL_BUS
	#pragma HLS INTERFACE m_axi depth=200 port=input offset=slave bundle=MASTER_BUS
	#pragma HLS INTERFACE m_axi depth=200 port=weight offset=slave bundle=MASTER_BUS
	#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

	int inp[U], wt[U], mult[U], add1_arr[add1], add2_arr[add2], k;

	#pragma HLS ARRAY_PARTITION variable=wt
	#pragma HLS ARRAY_PARTITION variable=inp
	#pragma HLS ARRAY_PARTITION variable=mult
	#pragma HLS ARRAY_PARTITION variable=add1_arr
	#pragma HLS ARRAY_PARTITION variable=add2_arr

	int acc = 0;

	L1: for(int i=0; i<accum_depth; i+=U)
	{
		#pragma HLS DEPENDENCE variable=mult intra false
		#pragma HLS DEPENDENCE variable=add1_arr intra false
		#pragma HLS DEPENDENCE variable=add2_arr intra false
		#pragma HLS PIPELINE II=1

		L2: for(int j=0; j<U; j++)
		{
			#pragma HLS PIPELINE II=1
			k = i + j;
			inp[j] = (k < accum_depth)? (input[k] + input_offset) : 0;
			wt[j] = (k < accum_depth)? (weight[k] + weight_offset) : 0;
		}

		L3: for(int j=0; j<U; j++)
		{
			#pragma HLS UNROLL factor=8
			mult[j] = inp[j] * wt[j];
		}

		L4: for(int j=0; j<add1; j++)
		{
			//#pragma HLS UNROLL
			add1_arr[j] = mult[2*j] + mult[2*j + 1];
		}

		L5: for(int j=0; j<add2; j++)
		{
			//#pragma HLS UNROLL
			add2_arr[j] = add1_arr[2*j] + add1_arr[2*j + 1];
		}

		acc += add2_arr[0] + add2_arr[1];
	}

	return acc;
}

