__be32 nfs4_callback_recall(void *argp, void *resp,
			    struct cb_process_state *cps)
{
	struct cb_recallargs *args = argp;
	struct inode *inode;
	__be32 res;
	
	res = htonl(NFS4ERR_OP_NOT_IN_SESSION);
	if (!cps->clp) /* Always set for v4.0. Set in cb_sequence for v4.1 */
		goto out;

	dprintk_rcu("NFS: RECALL callback request from %s\n",
		rpc_peeraddr2str(cps->clp->cl_rpcclient, RPC_DISPLAY_ADDR));

	res = htonl(NFS4ERR_BADHANDLE);
	inode = nfs_delegation_find_inode(cps->clp, &args->fh);
	if (IS_ERR(inode)) {
		if (inode == ERR_PTR(-EAGAIN))
			res = htonl(NFS4ERR_DELAY);
		trace_nfs4_cb_recall(cps->clp, &args->fh, NULL,
				&args->stateid, -ntohl(res));
		goto out;
	}
	/* Set up a helper thread to actually return the delegation */
	switch (nfs_async_inode_return_delegation(inode, &args->stateid)) {
	case 0:
		res = 0;
		break;
	case -ENOENT:
		res = htonl(NFS4ERR_BAD_STATEID);
		break;
	default:
		res = htonl(NFS4ERR_RESOURCE);
	}
	trace_nfs4_cb_recall(cps->clp, &args->fh, inode,
			&args->stateid, -ntohl(res));
	nfs_iput_and_deactive(inode);
out:
	dprintk("%s: exit with status = %d\n", __func__, ntohl(res));
	return res;
}


// Source: callback_proc.c
// Lines 75-115
