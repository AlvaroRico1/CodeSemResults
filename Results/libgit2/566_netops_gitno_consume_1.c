int gitno_consume(gitno_buffer *buf, const char *ptr)
{
	size_t consumed;

	GIT_ASSERT(ptr - buf->data >= 0);
	GIT_ASSERT(ptr - buf->data <= (int) buf->len);

	consumed = ptr - buf->data;

	memmove(buf->data, ptr, buf->offset - consumed);
	memset(buf->data + buf->offset, 0x0, buf->len - buf->offset);
	buf->offset -= consumed;

	return 0;
}


// Source: netops.c
// Lines 63-77
