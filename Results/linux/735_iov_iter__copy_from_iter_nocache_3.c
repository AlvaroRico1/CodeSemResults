size_t _copy_from_iter_nocache(void *addr, size_t bytes, struct iov_iter *i)
{
	char *to = addr;
	if (unlikely(iov_iter_is_pipe(i))) {
		WARN_ON(1);
		return 0;
	}
	iterate_and_advance(i, bytes, v,
		__copy_from_user_inatomic_nocache((to += v.iov_len) - v.iov_len,
					 v.iov_base, v.iov_len),
		memcpy_from_page((to += v.bv_len) - v.bv_len, v.bv_page,
				 v.bv_offset, v.bv_len),
		memcpy((to += v.iov_len) - v.iov_len, v.iov_base, v.iov_len)
	)

	return bytes;
}


// Source: iov_iter.c
// Lines 784-800
