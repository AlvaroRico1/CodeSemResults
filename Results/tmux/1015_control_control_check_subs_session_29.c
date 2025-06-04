control_check_subs_session(struct client *c, struct control_sub *csub)
{
	struct session		*s = c->session;
	struct format_tree	*ft;
	char			*value;

	ft = format_create_defaults(NULL, c, s, NULL, NULL);
	value = format_expand(ft, csub->format);
	format_free(ft);

	if (csub->last != NULL && strcmp(value, csub->last) == 0) {
		free(value);
		return;
	}
	control_write(c,
	    "%%subscription-changed %s $%u - - - : %s",
	    csub->name, s->id, value);
	free(csub->last);
	csub->last = value;
}


// Source: control.c
// Lines 833-852
