static void reverse_elements(void **dst, ssize_t start, ssize_t end)
{
	while (start < end) {
		void *tmp = dst[start];
		dst[start] = dst[end];
		dst[end] = tmp;

		start++;
		end--;
	}


// Source: tsort.c
// Lines 108-117
