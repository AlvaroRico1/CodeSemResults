file_write_data(struct client_files *files, struct imsg *imsg)
{
	struct msg_write_data	*msg = imsg->data;
	size_t			 msglen = imsg->hdr.len - IMSG_HEADER_SIZE;
	struct client_file	 find, *cf;
	size_t			 size = msglen - sizeof *msg;

	if (msglen < sizeof *msg)
		fatalx("bad MSG_WRITE size");
	find.stream = msg->stream;
	if ((cf = RB_FIND(client_files, files, &find)) == NULL)
		fatalx("unknown stream number");
	log_debug("write %zu to file %d", size, cf->stream);

	if (cf->event != NULL)
		bufferevent_write(cf->event, msg + 1, size);
}


// Source: file.c
// Lines 599-615
