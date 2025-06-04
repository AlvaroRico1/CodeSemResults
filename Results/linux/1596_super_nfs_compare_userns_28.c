static int nfs_compare_userns(const struct nfs_server *old,
		const struct nfs_server *new)
{
	const struct user_namespace *oldns = &init_user_ns;
	const struct user_namespace *newns = &init_user_ns;

	if (old->client && old->client->cl_cred)
		oldns = old->client->cl_cred->user_ns;
	if (new->client && new->client->cl_cred)
		newns = new->client->cl_cred->user_ns;
	if (oldns != newns)
		return 0;
	return 1;
}


// Source: super.c
// Lines 2511-2524
