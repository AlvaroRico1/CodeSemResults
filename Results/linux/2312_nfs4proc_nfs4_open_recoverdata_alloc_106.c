static struct nfs4_opendata *nfs4_open_recoverdata_alloc(struct nfs_open_context *ctx,
		struct nfs4_state *state, enum open_claim_type4 claim)
{
	struct nfs4_opendata *opendata;

	opendata = nfs4_opendata_alloc(ctx->dentry, state->owner, 0, 0,
			NULL, claim, GFP_NOFS);
	if (opendata == NULL)
		return ERR_PTR(-ENOMEM);
	opendata->state = state;
	refcount_inc(&state->count);
	return opendata;
}


// Source: nfs4proc.c
// Lines 2043-2055
