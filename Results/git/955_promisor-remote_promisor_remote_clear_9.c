void promisor_remote_clear(struct promisor_remote_config *config)
{
	while (config->promisors) {
		struct promisor_remote *r = config->promisors;
		config->promisors = config->promisors->next;
		free(r);
	}


// Source: promisor-remote.c
// Lines 167-173
