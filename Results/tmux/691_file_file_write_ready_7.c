file_write_ready(struct client_files *files, struct imsg *imsg)
{
	struct msg_write_ready	*msg = imsg->data;
	size_t			 msglen = imsg->hdr.len - IMSG_HEADER_SIZE;
	struct client_file	 find, *cf;

	if (msglen != sizeof *msg)
		fatalx("bad MSG_WRITE_READY size");
	find.stream = msg->stream;
	if ((cf = RB_FIND(client_files, files, &find)) == NULL)
		return;
	if (msg->error != 0) {
		cf->error = msg->error;
		file_fire_done(cf);
	} else
		file_push(cf);
}


// Source: file.c
// Lines 758-774
