input_dcs_dispatch(struct input_ctx *ictx)
{
	struct screen_write_ctx	*sctx = &ictx->ctx;
	u_char			*buf = ictx->input_buf;
	size_t			 len = ictx->input_len;
	const char		 prefix[] = "tmux;";
	const u_int		 prefixlen = (sizeof prefix) - 1;

	if (ictx->flags & INPUT_DISCARD)
		return (0);

	log_debug("%s: \"%s\"", __func__, buf);

	if (len >= prefixlen && strncmp(buf, prefix, prefixlen) == 0)
		screen_write_rawstring(sctx, buf + prefixlen, len - prefixlen);

	return (0);
}


// Source: input.c
// Lines 2232-2249
