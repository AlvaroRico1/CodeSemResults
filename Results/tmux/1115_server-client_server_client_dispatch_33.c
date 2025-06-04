server_client_dispatch(struct imsg *imsg, void *arg)
{
	struct client	*c = arg;
	ssize_t		 datalen;
	struct session	*s;

	if (c->flags & CLIENT_DEAD)
		return;

	if (imsg == NULL) {
		server_client_lost(c);
		return;
	}

	datalen = imsg->hdr.len - IMSG_HEADER_SIZE;

	switch (imsg->hdr.type) {
	case MSG_IDENTIFY_CLIENTPID:
	case MSG_IDENTIFY_CWD:
	case MSG_IDENTIFY_ENVIRON:
	case MSG_IDENTIFY_FEATURES:
	case MSG_IDENTIFY_FLAGS:
	case MSG_IDENTIFY_LONGFLAGS:
	case MSG_IDENTIFY_STDIN:
	case MSG_IDENTIFY_STDOUT:
	case MSG_IDENTIFY_TERM:
	case MSG_IDENTIFY_TERMINFO:
	case MSG_IDENTIFY_TTYNAME:
	case MSG_IDENTIFY_DONE:
		server_client_dispatch_identify(c, imsg);
		break;
	case MSG_COMMAND:
		server_client_dispatch_command(c, imsg);
		break;
	case MSG_RESIZE:
		if (datalen != 0)
			fatalx("bad MSG_RESIZE size");

		if (c->flags & CLIENT_CONTROL)
			break;
		server_client_update_latest(c);
		tty_resize(&c->tty);
		recalculate_sizes();
		if (c->overlay_resize == NULL)
			server_client_clear_overlay(c);
		else
			c->overlay_resize(c, c->overlay_data);
		server_redraw_client(c);
		if (c->session != NULL)
			notify_client("client-resized", c);
		break;
	case MSG_EXITING:
		if (datalen != 0)
			fatalx("bad MSG_EXITING size");
		server_client_set_session(c, NULL);
		recalculate_sizes();
		tty_close(&c->tty);
		proc_send(c->peer, MSG_EXITED, -1, NULL, 0);
		break;
	case MSG_WAKEUP:
	case MSG_UNLOCK:
		if (datalen != 0)
			fatalx("bad MSG_WAKEUP size");

		if (!(c->flags & CLIENT_SUSPENDED))
			break;
		c->flags &= ~CLIENT_SUSPENDED;

		if (c->fd == -1 || c->session == NULL) /* exited already */
			break;
		s = c->session;

		if (gettimeofday(&c->activity_time, NULL) != 0)
			fatal("gettimeofday failed");

		tty_start_tty(&c->tty);
		server_redraw_client(c);
		recalculate_sizes();

		if (s != NULL)
			session_update_activity(s, &c->activity_time);
		break;
	case MSG_SHELL:
		if (datalen != 0)
			fatalx("bad MSG_SHELL size");

		server_client_dispatch_shell(c);
		break;
	case MSG_WRITE_READY:
		file_write_ready(&c->files, imsg);
		break;
	case MSG_READ:
		file_read_data(&c->files, imsg);
		break;
	case MSG_READ_DONE:
		file_read_done(&c->files, imsg);
		break;
	}
}


// Source: server-client.c
// Lines 2050-2148
