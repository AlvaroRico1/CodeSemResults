int write_packetized_from_fd_no_flush(int fd_in, int fd_out)
{
	char *buf = xmalloc(LARGE_PACKET_DATA_MAX);
	int err = 0;
	ssize_t bytes_to_write;

	while (!err) {
		bytes_to_write = xread(fd_in, buf, LARGE_PACKET_DATA_MAX);
		if (bytes_to_write < 0) {
			free(buf);
			return COPY_READ_ERROR;
		}
		if (bytes_to_write == 0)
			break;
		err = packet_write_gently(fd_out, buf, bytes_to_write);
	}
	free(buf);
	return err;
}


// Source: pkt-line.c
// Lines 308-326
