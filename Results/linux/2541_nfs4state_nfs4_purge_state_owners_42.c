void nfs4_purge_state_owners(struct nfs_server *server, struct list_head *head)
{
	struct nfs_client *clp = server->nfs_client;
	struct nfs4_state_owner *sp, *tmp;

	spin_lock(&clp->cl_lock);
	list_for_each_entry_safe(sp, tmp, &server->state_owners_lru, so_lru) {
		list_move(&sp->so_lru, head);
		nfs4_remove_state_owner_locked(sp);
	}
	spin_unlock(&clp->cl_lock);
}


// Source: nfs4state.c
// Lines 635-646
