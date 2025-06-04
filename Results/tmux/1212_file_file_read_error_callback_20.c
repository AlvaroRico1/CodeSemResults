file_read_error_callback(__unused struct bufferevent *bev, __unused short what,
    void *arg)
{
	struct client_file	*cf = arg;
	struct msg_read_done	 msg;

	log_debug("read error file %d", cf->stream);

	msg.stream = cf->stream;
	msg.error = 0;
	proc_send(cf->peer, MSG_READ_DONE, -1, &msg, sizeof msg);

	bufferevent_free(cf->event);
	close(cf->fd);
	RB_REMOVE(client_files, cf->tree, cf);
	file_free(cf);
}


// Source: file.c
// Lines 644-660
