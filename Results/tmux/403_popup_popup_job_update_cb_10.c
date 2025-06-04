popup_job_update_cb(struct job *job)
{
	struct popup_data	*pd = job_get_data(job);
	struct evbuffer		*evb = job_get_event(job)->input;
	struct client		*c = pd->c;
	struct screen		*s = &pd->s;
	void			*data = EVBUFFER_DATA(evb);
	size_t			 size = EVBUFFER_LENGTH(evb);

	if (size == 0)
		return;

	if (pd->md != NULL) {
		c->overlay_check = menu_check_cb;
		c->overlay_data = pd->md;
	} else {
		c->overlay_check = NULL;
		c->overlay_data = NULL;
	}
	input_parse_screen(pd->ictx, s, popup_init_ctx_cb, pd, data, size);
	c->overlay_check = popup_check_cb;
	c->overlay_data = pd;

	evbuffer_drain(evb, size);
}


// Source: popup.c
// Lines 588-612
