mpihelp_mul(mpi_ptr_t prodp, mpi_ptr_t up, mpi_size_t usize,
	    mpi_ptr_t vp, mpi_size_t vsize, mpi_limb_t *_result)
{
	mpi_ptr_t prod_endp = prodp + usize + vsize - 1;
	mpi_limb_t cy;
	struct karatsuba_ctx ctx;

	if (vsize < KARATSUBA_THRESHOLD) {
		mpi_size_t i;
		mpi_limb_t v_limb;

		if (!vsize) {
			*_result = 0;
			return 0;
		}

		/* Multiply by the first limb in V separately, as the result can be
		 * stored (not added) to PROD.  We also avoid a loop for zeroing.  */
		v_limb = vp[0];
		if (v_limb <= 1) {
			if (v_limb == 1)
				MPN_COPY(prodp, up, usize);
			else
				MPN_ZERO(prodp, usize);
			cy = 0;
		} else
			cy = mpihelp_mul_1(prodp, up, usize, v_limb);

		prodp[usize] = cy;
		prodp++;

		/* For each iteration in the outer loop, multiply one limb from
		 * U with one limb from V, and add it to PROD.  */
		for (i = 1; i < vsize; i++) {
			v_limb = vp[i];
			if (v_limb <= 1) {
				cy = 0;
				if (v_limb == 1)
					cy = mpihelp_add_n(prodp, prodp, up,
							   usize);
			} else
				cy = mpihelp_addmul_1(prodp, up, usize, v_limb);

			prodp[usize] = cy;
			prodp++;
		}

		*_result = cy;
		return 0;
	}

	memset(&ctx, 0, sizeof ctx);
	if (mpihelp_mul_karatsuba_case(prodp, up, usize, vp, vsize, &ctx) < 0)
		return -ENOMEM;
	mpihelp_release_karatsuba_ctx(&ctx);
	*_result = *prod_endp;
	return 0;
}


// Source: mpih-mul.c
// Lines 427-484
