int git_fetch_download_pack(git_remote *remote)
{
	git_transport *t = remote->transport;

	if (!remote->need_pack)
		return 0;

	return t->download_pack(t, remote->repo, &remote->stats);
}


// Source: fetch.c
// Lines 192-200
