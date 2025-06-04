static int recv_pkt(git_pkt **out_pkt, git_pkt_type *out_type, gitno_buffer *buf)
{
	const char *ptr = buf->data, *line_end = ptr;
	git_pkt *pkt = NULL;
	int error = 0, ret;

	do {
		if (buf->offset > 0)
			error = git_pkt_parse_line(&pkt, &line_end, ptr, buf->offset);
		else
			error = GIT_EBUFS;

		if (error == 0)
			break; /* return the pkt */

		if (error < 0 && error != GIT_EBUFS)
			return error;

		if ((ret = gitno_recv(buf)) < 0) {
			return ret;
		} else if (ret == 0) {
			git_error_set(GIT_ERROR_NET, "early EOF");
			return GIT_EEOF;
		}
	} while (error);

	if (gitno_consume(buf, line_end) < 0)
		return -1;

	if (out_type != NULL)
		*out_type = pkt->type;
	if (out_pkt != NULL)
		*out_pkt = pkt;
	else
		git__free(pkt);

	return error;
}


// Source: smart_protocol.c
// Lines 227-264
