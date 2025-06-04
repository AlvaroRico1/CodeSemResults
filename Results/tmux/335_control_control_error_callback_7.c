control_error_callback(__unused struct bufferevent *bufev,
    __unused short what, void *data)
{
	struct client	*c = data;

	c->flags |= CLIENT_EXIT;
}


// Source: control.c
// Lines 537-543
