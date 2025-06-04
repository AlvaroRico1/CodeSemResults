static void promisor_remote_init(struct repository *r)
{
	struct promisor_remote_config *config;

	if (r->promisor_remote_config)
		return;
	config = r->promisor_remote_config =
		xcalloc(sizeof(*r->promisor_remote_config), 1);
	config->promisors_tail = &config->promisors;

	repo_config(r, promisor_remote_config, config);

	if (r->repository_format_partial_clone) {
		struct promisor_remote *o, *previous;

		o = promisor_remote_lookup(config,
					   r->repository_format_partial_clone,
					   &previous);
		if (o)
			promisor_remote_move_to_tail(config, o, previous);
		else
			promisor_remote_new(config, r->repository_format_partial_clone);
	}
}


// Source: promisor-remote.c
// Lines 142-165
