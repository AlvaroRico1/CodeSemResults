struct nfs4_state_owner *nfs4_get_state_owner(struct nfs_server *server,
					      const struct cred *cred,
					      gfp_t gfp_flags)
{
	struct nfs_client *clp = server->nfs_client;
	struct nfs4_state_owner *sp, *new;

	spin_lock(&clp->cl_lock);
	sp = nfs4_find_state_owner_locked(server, cred);
	spin_unlock(&clp->cl_lock);
	if (sp != NULL)
		goto out;
	new = nfs4_alloc_state_owner(server, cred, gfp_flags);
	if (new == NULL)
		goto out;
	spin_lock(&clp->cl_lock);
	sp = nfs4_insert_state_owner_locked(new);
	spin_unlock(&clp->cl_lock);
	if (sp != new)
		nfs4_free_state_owner(new);
out:
	nfs4_gc_state_owners(server);
	return sp;
}


// Source: nfs4state.c
// Lines 574-597
