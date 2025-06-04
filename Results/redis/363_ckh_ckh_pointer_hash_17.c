ckh_pointer_hash(const void *key, size_t r_hash[2]) {
	union {
		const void	*v;
		size_t		i;
	} u;


// Source: ckh.c
// Lines 555-559
