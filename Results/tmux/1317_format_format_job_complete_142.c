format_job_complete(struct job *job)
{
	struct format_job	*fj = job_get_data(job);
	struct evbuffer		*evb = job_get_event(job)->input;
	char			*line, *buf;
	size_t			 len;

	fj->job = NULL;

	buf = NULL;
	if ((line = evbuffer_readline(evb)) == NULL) {
		len = EVBUFFER_LENGTH(evb);
		buf = xmalloc(len + 1);
		if (len != 0)
			memcpy(buf, EVBUFFER_DATA(evb), len);
		buf[len] = '\0';
	} else
		buf = line;

	log_debug("%s: %p %s: %s", __func__, fj, fj->cmd, buf);

	if (*buf != '\0' || !fj->updated) {
		free(fj->out);
		fj->out = buf;
	} else
		free(buf);

	if (fj->status) {
		if (fj->client != NULL)
			server_status_client(fj->client);
		fj->status = 0;
	}
}


// Source: format.c
// Lines 312-344
