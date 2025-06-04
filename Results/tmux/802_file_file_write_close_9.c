file_write_close(struct client_files *files, struct imsg *imsg)
{
	struct msg_write_close	*msg = imsg->data;
	size_t			 msglen = imsg->hdr.len - IMSG_HEADER_SIZE;
	struct client_file	 find, *cf;

	if (msglen != sizeof *msg)
		fatalx("bad MSG_WRITE_CLOSE size");
	find.stream = msg->stream;
	if ((cf = RB_FIND(client_files, files, &find)) == NULL)
		fatalx("unknown stream number");
	log_debug("close file %d", cf->stream);

	if (cf->event == NULL || EVBUFFER_LENGTH(cf->event->output) == 0) {
		if (cf->event != NULL)
			bufferevent_free(cf->event);
		if (cf->fd != -1)
			close(cf->fd);
		RB_REMOVE(client_files, files, cf);
		file_free(cf);
	}
}


// Source: file.c
// Lines 619-640
