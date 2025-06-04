cmd_refresh_client_update_subscription(struct client *tc, const char *value)
{
	char			*copy, *split, *name, *what;
	enum control_sub_type	 subtype;
	int			 subid = -1;

	copy = name = xstrdup(value);
	if ((split = strchr(copy, ':')) == NULL) {
		control_remove_sub(tc, copy);
		goto out;
	}
	*split++ = '\0';

	what = split;
	if ((split = strchr(what, ':')) == NULL)
		goto out;
	*split++ = '\0';

	if (strcmp(what, "%*") == 0)
		subtype = CONTROL_SUB_ALL_PANES;
	else if (sscanf(what, "%%%d", &subid) == 1 && subid >= 0)
		subtype = CONTROL_SUB_PANE;
	else if (strcmp(what, "@*") == 0)
		subtype = CONTROL_SUB_ALL_WINDOWS;
	else if (sscanf(what, "@%d", &subid) == 1 && subid >= 0)
		subtype = CONTROL_SUB_WINDOW;
	else
		subtype = CONTROL_SUB_SESSION;
	control_add_sub(tc, name, subtype, subid, split);

out:
	free(copy);
}


// Source: cmd-refresh-client.c
// Lines 46-78
