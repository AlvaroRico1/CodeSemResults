file_read_open(struct client_files *files, struct tmuxpeer *peer,
    struct imsg *imsg, int allow_streams, int close_received, client_file_cb cb,
    void *cbdata)
{
	struct msg_read_open	*msg = imsg->data;
	size_t			 msglen = imsg->hdr.len - IMSG_HEADER_SIZE;
	const char		*path;
	struct msg_read_done	 reply;
	struct client_file	 find, *cf;
	const int		 flags = O_NONBLOCK|O_RDONLY;
	int			 error;

	if (msglen < sizeof *msg)
		fatalx("bad MSG_READ_OPEN size");
	if (msglen == sizeof *msg)
		path = "-";
	else
		path = (const char *)(msg + 1);
	log_debug("open read file %d %s", msg->stream, path);

	find.stream = msg->stream;
	if (RB_FIND(client_files, files, &find) != NULL) {
		error = EBADF;
		goto reply;
	}
	cf = file_create_with_peer(peer, files, msg->stream, cb, cbdata);
	if (cf->closed) {
		error = EBADF;
		goto reply;
	}

	cf->fd = -1;
	if (msg->fd == -1)
		cf->fd = open(path, flags);
	else if (allow_streams) {
		if (msg->fd != STDIN_FILENO)
			errno = EBADF;
		else {
			cf->fd = dup(msg->fd);
			if (close_received)
				close(msg->fd); /* can only be used once */
		}
	} else
		errno = EBADF;
	if (cf->fd == -1) {
		error = errno;
		goto reply;
	}

	cf->event = bufferevent_new(cf->fd, file_read_callback, NULL,
	    file_read_error_callback, cf);
	bufferevent_enable(cf->event, EV_READ);
	return;

reply:
	reply.stream = msg->stream;
	reply.error = error;
	proc_send(peer, MSG_READ_DONE, -1, &reply, sizeof reply);
}


// Source: file.c
// Lines 696-754
