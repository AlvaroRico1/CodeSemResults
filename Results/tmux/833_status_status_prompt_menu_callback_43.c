status_prompt_menu_callback(__unused struct menu *menu, u_int idx, key_code key,
    void *data)
{
	struct status_prompt_menu	*spm = data;
	struct client			*c = spm->c;
	u_int				 i;
	char				*s;

	if (key != KEYC_NONE) {
		idx += spm->start;
		if (spm->flag == '\0')
			s = xstrdup(spm->list[idx]);
		else
			xasprintf(&s, "-%c%s", spm->flag, spm->list[idx]);
		if (c->prompt_type == PROMPT_TYPE_WINDOW_TARGET) {
			free(c->prompt_buffer);
			c->prompt_buffer = utf8_fromcstr(s);
			c->prompt_index = utf8_strlen(c->prompt_buffer);
			c->flags |= CLIENT_REDRAWSTATUS;
		} else if (status_prompt_replace_complete(c, s))
			c->flags |= CLIENT_REDRAWSTATUS;
		free(s);
	}

	for (i = 0; i < spm->size; i++)
		free(spm->list[i]);
	free(spm->list);
}


// Source: status.c
// Lines 1654-1681
