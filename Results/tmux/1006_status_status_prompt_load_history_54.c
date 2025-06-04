status_prompt_load_history(void)
{
	FILE	*f;
	char	*history_file, *line, *tmp;
	size_t	 length;

	if ((history_file = status_prompt_find_history_file()) == NULL)
		return;
	log_debug("loading history from %s", history_file);

	f = fopen(history_file, "r");
	if (f == NULL) {
		log_debug("%s: %s", history_file, strerror(errno));
		free(history_file);
		return;
	}
	free(history_file);

	for (;;) {
		if ((line = fgetln(f, &length)) == NULL)
			break;

		if (length > 0) {
			if (line[length - 1] == '\n') {
				line[length - 1] = '\0';
				status_prompt_add_typed_history(line);
			} else {
				tmp = xmalloc(length + 1);
				memcpy(tmp, line, length);
				tmp[length] = '\0';
				status_prompt_add_typed_history(tmp);
				free(tmp);
			}
		}
	}
	fclose(f);
}


// Source: status.c
// Lines 108-144
