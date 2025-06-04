server_client_resize_timer(__unused int fd, __unused short events, void *data)
{
	struct window_pane	*wp = data;

	log_debug("%s: %%%u resize timer expired", __func__, wp->id);
	evtimer_del(&wp->resize_timer);
}


// Source: server-client.c
// Lines 1521-1527
