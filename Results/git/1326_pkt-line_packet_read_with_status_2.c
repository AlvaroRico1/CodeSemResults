enum packet_read_status packet_read_with_status(int fd, char **src_buffer,
						size_t *src_len, char *buffer,
						unsigned size, int *pktlen,
						int options)
{
	int len;
	char linelen[4];

	if (get_packet_data(fd, src_buffer, src_len, linelen, 4, options) < 0) {
		*pktlen = -1;
		return PACKET_READ_EOF;
	}

	len = packet_length(linelen);

	if (len < 0) {
		if (options & PACKET_READ_GENTLE_ON_READ_ERROR)
			return error(_("protocol error: bad line length "
				       "character: %.4s"), linelen);
		die(_("protocol error: bad line length character: %.4s"), linelen);
	} else if (!len) {
		packet_trace("0000", 4, 0);
		*pktlen = 0;
		return PACKET_READ_FLUSH;
	} else if (len == 1) {
		packet_trace("0001", 4, 0);
		*pktlen = 0;
		return PACKET_READ_DELIM;
	} else if (len == 2) {
		packet_trace("0002", 4, 0);
		*pktlen = 0;
		return PACKET_READ_RESPONSE_END;
	} else if (len < 4) {
		if (options & PACKET_READ_GENTLE_ON_READ_ERROR)
			return error(_("protocol error: bad line length %d"),
				     len);
		die(_("protocol error: bad line length %d"), len);
	}

	len -= 4;
	if ((unsigned)len >= size) {
		if (options & PACKET_READ_GENTLE_ON_READ_ERROR)
			return error(_("protocol error: bad line length %d"),
				     len);
		die(_("protocol error: bad line length %d"), len);
	}

	if (get_packet_data(fd, src_buffer, src_len, buffer, len, options) < 0) {
		*pktlen = -1;
		return PACKET_READ_EOF;
	}

	if ((options & PACKET_READ_CHOMP_NEWLINE) &&
	    len && buffer[len-1] == '\n')
		len--;

	buffer[len] = 0;
	packet_trace(buffer, len, 0);

	if ((options & PACKET_READ_DIE_ON_ERR_PACKET) &&
	    starts_with(buffer, "ERR "))
		die(_("remote error: %s"), buffer + 4);

	*pktlen = len;
	return PACKET_READ_NORMAL;
}


// Source: pkt-line.c
// Lines 389-454
