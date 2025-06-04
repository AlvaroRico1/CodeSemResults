struct nfs_seqid *nfs_alloc_seqid(struct nfs_seqid_counter *counter, gfp_t gfp_mask)
{
	struct nfs_seqid *new;

	new = kmalloc(sizeof(*new), gfp_mask);
	if (new == NULL)
		return ERR_PTR(-ENOMEM);
	new->sequence = counter;
	INIT_LIST_HEAD(&new->list);
	new->task = NULL;
	return new;
}


// Source: nfs4state.c
// Lines 1089-1100
