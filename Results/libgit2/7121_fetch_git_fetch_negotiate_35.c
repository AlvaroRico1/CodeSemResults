int git_fetch_negotiate(git_remote *remote, const git_fetch_options *opts)
{
	git_transport *t = remote->transport;

	remote->need_pack = 0;

	if (filter_wants(remote, opts) < 0)
		return -1;

	/* Don't try to negotiate when we don't want anything */
	if (!remote->need_pack)
		return 0;

	/*
	 * Now we have everything set up so we can start tell the
	 * server what we want and what we have.
	 */
	return t->negotiate_fetch(t,
		remote->repo,
		(const git_remote_head * const *)remote->refs.contents,
		remote->refs.length);
}


// Source: fetch.c
// Lines 169-190
