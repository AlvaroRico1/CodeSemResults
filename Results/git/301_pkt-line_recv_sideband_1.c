int recv_sideband(const char *me, int in_stream, int out)
{
	char buf[LARGE_PACKET_MAX + 1];
	int len;
	struct strbuf scratch = STRBUF_INIT;
	enum sideband_type sideband_type;

	while (1) {
		int status = packet_read_with_status(in_stream, NULL, NULL,
						     buf, LARGE_PACKET_MAX,
						     &len,
						     PACKET_READ_GENTLE_ON_EOF);
		if (!demultiplex_sideband(me, status, buf, len, 0, &scratch,
					  &sideband_type))
			continue;
		switch (sideband_type) {
		case SIDEBAND_PRIMARY:
			write_or_die(out, buf + 1, len - 1);
			break;
		default: /* errors: message already written */
			if (scratch.len > 0)
				BUG("unhandled incomplete sideband: '%s'",
				    scratch.buf);
			return sideband_type;
		}
	}


// Source: pkt-line.c
// Lines 533-558
