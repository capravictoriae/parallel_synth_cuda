__kernel void VectorAdd(__global int* c, __global int* a, __global int* b)

{

	// Index of the elements to add
	unsigned int n = get_global_id(0);

	// Sum the nth element of vectors a and b and store in c

	c[n] = a[n] + b[n];

}