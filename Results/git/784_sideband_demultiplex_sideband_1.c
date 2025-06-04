int demultiplex_sideband(const char *me, int status,
			 char *buf, int len,
			 int die_on_error,
			 struct strbuf *scratch,
			 enum sideband_type *sideband_type)
{
	static const char *suffix;
	const char *b, *brk;
	int band;

	if (!suffix) {
		if (isatty(2) && !is_terminal_dumb())
			suffix = ANSI_SUFFIX;
		else
			suffix = DUMB_SUFFIX;
	}

	if (status == PACKET_READ_EOF) {
		strbuf_addf(scratch,
			    "%s%s: unexpected disconnect while reading sideband packet",
			    scratch->len ? "\n" : "", me);
		*sideband_type = SIDEBAND_PROTOCOL_ERROR;
		goto cleanup;
	}

	if (len < 0)
		BUG("negative length on non-eof packet read");

	if (len == 0) {
		if (status == PACKET_READ_NORMAL) {
			strbuf_addf(scratch,
				    "%s%s: protocol error: missing sideband designator",
				    scratch->len ? "\n" : "", me);
			*sideband_type = SIDEBAND_PROTOCOL_ERROR;
		} else {
			/* covers flush, delim, etc */
			*sideband_type = SIDEBAND_FLUSH;
		}
		goto cleanup;
	}

	band = buf[0] & 0xff;
	buf[len] = '\0';
	len--;
	switch (band) {
	case 3:
		if (die_on_error)
			die(_("remote error: %s"), buf + 1);
		strbuf_addf(scratch, "%s%s", scratch->len ? "\n" : "",
			    DISPLAY_PREFIX);
		maybe_colorize_sideband(scratch, buf + 1, len);

		*sideband_type = SIDEBAND_REMOTE_ERROR;
		break;
	case 2:
		b = buf + 1;

		/*
		 * Append a suffix to each nonempty line to clear the
		 * end of the screen line.
		 *
		 * The output is accumulated in a buffer and
		 * each line is printed to stderr using
		 * write(2) to ensure inter-process atomicity.
		 */
		while ((brk = strpbrk(b, "\n\r"))) {
			int linelen = brk - b;

			/*
			 * For message accross packet boundary, there would have
			 * a nonempty "scratch" buffer from last call of this
			 * function, and there may have a leading CR/LF in "buf".
			 * For this case we should add a clear-to-eol suffix to
			 * clean leftover letters we previously have written on
			 * the same line.
			 */
			if (scratch->len && !linelen)
				strbuf_addstr(scratch, suffix);

			if (!scratch->len)
				strbuf_addstr(scratch, DISPLAY_PREFIX);

			/*
			 * A use case that we should not add clear-to-eol suffix
			 * to empty lines:
			 *
			 * For progress reporting we may receive a bunch of
			 * percentage updates followed by '\r' to remain on the
			 * same line, and at the end receive a single '\n' to
			 * move to the next line. We should preserve the final
			 * status report line by not appending clear-to-eol
			 * suffix to this single line break.
			 */
			if (linelen > 0) {
				maybe_colorize_sideband(scratch, b, linelen);
				strbuf_addstr(scratch, suffix);
			}

			strbuf_addch(scratch, *brk);
			xwrite(2, scratch->buf, scratch->len);
			strbuf_reset(scratch);

			b = brk + 1;
		}

		if (*b) {
			strbuf_addstr(scratch, scratch->len ?
				    "" : DISPLAY_PREFIX);
			maybe_colorize_sideband(scratch, b, strlen(b));
		}
		return 0;
	case 1:
		*sideband_type = SIDEBAND_PRIMARY;
		return 1;
	default:
		strbuf_addf(scratch, "%s%s: protocol error: bad band #%d",
			    scratch->len ? "\n" : "", me, band);
		*sideband_type = SIDEBAND_PROTOCOL_ERROR;
		break;
	}

cleanup:
	if (die_on_error && *sideband_type == SIDEBAND_PROTOCOL_ERROR)
		die("%s", scratch->buf);
	if (scratch->len) {
		strbuf_addch(scratch, '\n');
		xwrite(2, scratch->buf, scratch->len);
	}
	strbuf_release(scratch);
	return 1;
}


// Source: sideband.c
// Lines 118-248
