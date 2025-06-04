static struct promisor_remote *promisor_remote_lookup(struct promisor_remote_config *config,
						      const char *remote_name,
						      struct promisor_remote **previous)
{
	struct promisor_remote *r, *p;

	for (p = NULL, r = config->promisors; r; p = r, r = r->next)
		if (!strcmp(r->name, remote_name)) {
			if (previous)
				*previous = p;
			return r;
		}

	return NULL;
}


// Source: promisor-remote.c
// Lines 67-81
