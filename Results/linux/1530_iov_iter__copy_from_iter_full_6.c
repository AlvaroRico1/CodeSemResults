bool _copy_from_iter_full(void *addr, size_t bytes, struct iov_iter *i)
{
	char *to = addr;
	if (unlikely(iov_iter_is_pipe(i))) {
		WARN_ON(1);
		return false;
	}
	if (unlikely(i->count < bytes))
		return false;

	if (iter_is_iovec(i))
		might_fault();
	iterate_all_kinds(i, bytes, v, ({
		if (copyin((to += v.iov_len) - v.iov_len,
				      v.iov_base, v.iov_len))
			return false;
		0;}),
		memcpy_from_page((to += v.bv_len) - v.bv_len, v.bv_page,
				 v.bv_offset, v.bv_len),
		memcpy((to += v.iov_len) - v.iov_len, v.iov_base, v.iov_len)
	)

	iov_iter_advance(i, bytes);
	return true;
}


// Source: iov_iter.c
// Lines 757-781
