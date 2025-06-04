int mpi_write_to_sgl(MPI a, struct scatterlist *sgl, unsigned nbytes,
		     int *sign)
{
	u8 *p, *p2;
#if BYTES_PER_MPI_LIMB == 4
	__be32 alimb;
#elif BYTES_PER_MPI_LIMB == 8
	__be64 alimb;
#else
#error please implement for this limb size.
#endif
	unsigned int n = mpi_get_size(a);
	struct sg_mapping_iter miter;
	int i, x, buf_len;
	int nents;

	if (sign)
		*sign = a->sign;

	if (nbytes < n)
		return -EOVERFLOW;

	nents = sg_nents_for_len(sgl, nbytes);
	if (nents < 0)
		return -EINVAL;

	sg_miter_start(&miter, sgl, nents, SG_MITER_ATOMIC | SG_MITER_TO_SG);
	sg_miter_next(&miter);
	buf_len = miter.length;
	p2 = miter.addr;

	while (nbytes > n) {
		i = min_t(unsigned, nbytes - n, buf_len);
		memset(p2, 0, i);
		p2 += i;
		nbytes -= i;

		buf_len -= i;
		if (!buf_len) {
			sg_miter_next(&miter);
			buf_len = miter.length;
			p2 = miter.addr;
		}
	}

	for (i = a->nlimbs - 1; i >= 0; i--) {
#if BYTES_PER_MPI_LIMB == 4
		alimb = a->d[i] ? cpu_to_be32(a->d[i]) : 0;
#elif BYTES_PER_MPI_LIMB == 8
		alimb = a->d[i] ? cpu_to_be64(a->d[i]) : 0;
#else
#error please implement for this limb size.
#endif
		p = (u8 *)&alimb;

		for (x = 0; x < sizeof(alimb); x++) {
			*p2++ = *p++;
			if (!--buf_len) {
				sg_miter_next(&miter);
				buf_len = miter.length;
				p2 = miter.addr;
			}
		}
	}

	sg_miter_stop(&miter);
	return 0;
}


// Source: mpicoder.c
// Lines 247-314
