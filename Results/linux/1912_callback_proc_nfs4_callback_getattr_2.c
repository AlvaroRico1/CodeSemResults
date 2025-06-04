__be32 nfs4_callback_getattr(void *argp, void *resp,
			     struct cb_process_state *cps)
{
	struct cb_getattrargs *args = argp;
	struct cb_getattrres *res = resp;
	struct nfs_delegation *delegation;
	struct nfs_inode *nfsi;
	struct inode *inode;

	res->status = htonl(NFS4ERR_OP_NOT_IN_SESSION);
	if (!cps->clp) /* Always set for v4.0. Set in cb_sequence for v4.1 */
		goto out;

	res->bitmap[0] = res->bitmap[1] = 0;
	res->status = htonl(NFS4ERR_BADHANDLE);

	dprintk_rcu("NFS: GETATTR callback request from %s\n",
		rpc_peeraddr2str(cps->clp->cl_rpcclient, RPC_DISPLAY_ADDR));

	inode = nfs_delegation_find_inode(cps->clp, &args->fh);
	if (IS_ERR(inode)) {
		if (inode == ERR_PTR(-EAGAIN))
			res->status = htonl(NFS4ERR_DELAY);
		trace_nfs4_cb_getattr(cps->clp, &args->fh, NULL,
				-ntohl(res->status));
		goto out;
	}
	nfsi = NFS_I(inode);
	rcu_read_lock();
	delegation = rcu_dereference(nfsi->delegation);
	if (delegation == NULL || (delegation->type & FMODE_WRITE) == 0)
		goto out_iput;
	res->size = i_size_read(inode);
	res->change_attr = delegation->change_attr;
	if (nfs_have_writebacks(inode))
		res->change_attr++;
	res->ctime = timespec64_to_timespec(inode->i_ctime);
	res->mtime = timespec64_to_timespec(inode->i_mtime);
	res->bitmap[0] = (FATTR4_WORD0_CHANGE|FATTR4_WORD0_SIZE) &
		args->bitmap[0];
	res->bitmap[1] = (FATTR4_WORD1_TIME_METADATA|FATTR4_WORD1_TIME_MODIFY) &
		args->bitmap[1];
	res->status = 0;
out_iput:
	rcu_read_unlock();
	trace_nfs4_cb_getattr(cps->clp, &args->fh, inode, -ntohl(res->status));
	nfs_iput_and_deactive(inode);
out:
	dprintk("%s: exit with status = %d\n", __func__, ntohl(res->status));
	return res->status;
}


// Source: callback_proc.c
// Lines 23-73
