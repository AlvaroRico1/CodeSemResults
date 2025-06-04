void nfs4_put_state_owner(struct nfs4_state_owner *sp)
{
	struct nfs_server *server = sp->so_server;
	struct nfs_client *clp = server->nfs_client;

	if (!atomic_dec_and_lock(&sp->so_count, &clp->cl_lock))
		return;

	sp->so_expires = jiffies;
	list_add_tail(&sp->so_lru, &server->state_owners_lru);
	spin_unlock(&clp->cl_lock);
}


// Source: nfs4state.c
// Lines 611-622
