static struct promisor_remote *promisor_remote_new(struct promisor_remote_config *config,
						   const char *remote_name)
{
	struct promisor_remote *r;

	if (*remote_name == '/') {
		warning(_("promisor remote name cannot begin with '/': %s"),
			remote_name);
		return NULL;
	}

	FLEX_ALLOC_STR(r, name, remote_name);

	*config->promisors_tail = r;
	config->promisors_tail = &r->next;

	return r;
}


// Source: promisor-remote.c
// Lines 48-65
