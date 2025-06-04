size_t _copy_to_iter_mcsafe(const void *addr, size_t bytes, struct iov_iter *i)
{
	const char *from = addr;
	unsigned long rem, curr_addr, s_addr = (unsigned long) addr;

	if (unlikely(iov_iter_is_pipe(i)))
		return copy_pipe_to_iter_mcsafe(addr, bytes, i);
	if (iter_is_iovec(i))
		might_fault();
	iterate_and_advance(i, bytes, v,
		copyout_mcsafe(v.iov_base, (from += v.iov_len) - v.iov_len, v.iov_len),
		({
		rem = memcpy_mcsafe_to_page(v.bv_page, v.bv_offset,
                               (from += v.bv_len) - v.bv_len, v.bv_len);
		if (rem) {
			curr_addr = (unsigned long) from;
			bytes = curr_addr - s_addr - rem;
			return bytes;
		}
		}),
		({
		rem = memcpy_mcsafe(v.iov_base, (from += v.iov_len) - v.iov_len,
				v.iov_len);
		if (rem) {
			curr_addr = (unsigned long) from;
			bytes = curr_addr - s_addr - rem;
			return bytes;
		}
		})
	)

	return bytes;
}


// Source: iov_iter.c
// Lines 701-733
