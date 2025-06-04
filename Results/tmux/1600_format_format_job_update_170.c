format_job_update(struct job *job)
{
	struct format_job	*fj = job_get_data(job);
	struct evbuffer		*evb = job_get_event(job)->input;
	char			*line = NULL, *next;
	time_t			 t;

	while ((next = evbuffer_readline(evb)) != NULL) {
		free(line);
		line = next;
	}
	if (line == NULL)
		return;
	fj->updated = 1;

	free(fj->out);
	fj->out = line;

	log_debug("%s: %p %s: %s", __func__, fj, fj->cmd, fj->out);

	t = time(NULL);
	if (fj->status && fj->last != t) {
		if (fj->client != NULL)
			server_status_client(fj->client);
		fj->last = t;
	}
}


// Source: format.c
// Lines 282-308
