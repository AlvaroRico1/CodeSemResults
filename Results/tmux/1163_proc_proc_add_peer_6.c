proc_add_peer(struct tmuxproc *tp, int fd,
    void (*dispatchcb)(struct imsg *, void *), void *arg)
{
	struct tmuxpeer	*peer;

	peer = xcalloc(1, sizeof *peer);
	peer->parent = tp;

	peer->dispatchcb = dispatchcb;
	peer->arg = arg;

	imsg_init(&peer->ibuf, fd);
	event_set(&peer->event, fd, EV_READ, proc_event_cb, peer);

	log_debug("add peer %p: %d (%p)", peer, fd, arg);
	TAILQ_INSERT_TAIL(&tp->peers, peer, entry);

	proc_update_event(peer);
	return (peer);
}


// Source: proc.c
// Lines 307-326
