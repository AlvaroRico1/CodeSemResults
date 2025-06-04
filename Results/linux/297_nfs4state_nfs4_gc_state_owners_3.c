static void nfs4_gc_state_owners(struct nfs_server *server)
{
	struct nfs_client *clp = server->nfs_client;
	struct nfs4_state_owner *sp, *tmp;
	unsigned long time_min, time_max;
	LIST_HEAD(doomed);

	spin_lock(&clp->cl_lock);
	time_max = jiffies;
	time_min = (long)time_max - (long)clp->cl_lease_time;
	list_for_each_entry_safe(sp, tmp, &server->state_owners_lru, so_lru) {
		/* NB: LRU is sorted so that oldest is at the head */
		if (time_in_range(sp->so_expires, time_min, time_max))
			break;
		list_move(&sp->so_lru, &doomed);
		nfs4_remove_state_owner_locked(sp);
	}
	spin_unlock(&clp->cl_lock);

	list_for_each_entry_safe(sp, tmp, &doomed, so_lru) {
		list_del(&sp->so_lru);
		nfs4_free_state_owner(sp);
	}
}


// Source: nfs4state.c
// Lines 541-564
