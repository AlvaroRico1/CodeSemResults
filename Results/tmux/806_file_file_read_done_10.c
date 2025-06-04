file_read_done(struct client_files *files, struct imsg *imsg)
{
	struct msg_read_done	*msg = imsg->data;
	size_t			 msglen = imsg->hdr.len - IMSG_HEADER_SIZE;
	struct client_file	 find, *cf;

	if (msglen != sizeof *msg)
		fatalx("bad MSG_READ_DONE size");
	find.stream = msg->stream;
	if ((cf = RB_FIND(client_files, files, &find)) == NULL)
		return;

	log_debug("file %d read done", cf->stream);
	cf->error = msg->error;
	file_fire_done(cf);
}


// Source: file.c
// Lines 804-819
