static void decode_header(struct mailinfo *mi, struct strbuf *it)
{
	char *in, *ep, *cp;
	struct strbuf outbuf = STRBUF_INIT, *dec;
	struct strbuf charset_q = STRBUF_INIT, piecebuf = STRBUF_INIT;
	int found_error = 1; /* pessimism */

	in = it->buf;
	while (in - it->buf <= it->len && (ep = strstr(in, "=?")) != NULL) {
		int encoding;
		strbuf_reset(&charset_q);
		strbuf_reset(&piecebuf);

		if (in != ep) {
			/*
			 * We are about to process an encoded-word
			 * that begins at ep, but there is something
			 * before the encoded word.
			 */
			char *scan;
			for (scan = in; scan < ep; scan++)
				if (!isspace(*scan))
					break;

			if (scan != ep || in == it->buf) {
				/*
				 * We should not lose that "something",
				 * unless we have just processed an
				 * encoded-word, and there is only LWS
				 * before the one we are about to process.
				 */
				strbuf_add(&outbuf, in, ep - in);
			}
		}
		/* E.g.
		 * ep : "=?iso-2022-jp?B?GyR...?= foo"
		 * ep : "=?ISO-8859-1?Q?Foo=FCbar?= baz"
		 */
		ep += 2;

		if (ep - it->buf >= it->len || !(cp = strchr(ep, '?')))
			goto release_return;

		if (cp + 3 - it->buf > it->len)
			goto release_return;
		strbuf_add(&charset_q, ep, cp - ep);

		encoding = cp[1];
		if (!encoding || cp[2] != '?')
			goto release_return;
		ep = strstr(cp + 3, "?=");
		if (!ep)
			goto release_return;
		strbuf_add(&piecebuf, cp + 3, ep - cp - 3);
		switch (tolower(encoding)) {
		default:
			goto release_return;
		case 'b':
			dec = decode_b_segment(&piecebuf);
			break;
		case 'q':
			dec = decode_q_segment(&piecebuf, 1);
			break;
		}
		if (convert_to_utf8(mi, dec, charset_q.buf))
			goto release_return;

		strbuf_addbuf(&outbuf, dec);
		strbuf_release(dec);
		free(dec);
		in = ep + 2;
	}
	strbuf_addstr(&outbuf, in);
	strbuf_reset(it);
	strbuf_addbuf(it, &outbuf);
	found_error = 0;
release_return:
	strbuf_release(&outbuf);
	strbuf_release(&charset_q);
	strbuf_release(&piecebuf);

	if (found_error)
		mi->input_error = -1;
}


// Source: mailinfo.c
// Lines 468-551
