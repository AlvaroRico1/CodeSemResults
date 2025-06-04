static void nfs4_delegreturn_release(void *calldata)
{
	struct nfs4_delegreturndata *data = calldata;
	struct inode *inode = data->inode;

	if (inode) {
		if (data->lr.roc)
			pnfs_roc_release(&data->lr.arg, &data->lr.res,
					data->res.lr_ret);
		nfs_post_op_update_inode_force_wcc(inode, &data->fattr);
		nfs_iput_and_deactive(inode);
	}
	kfree(calldata);
}


// Source: nfs4proc.c
// Lines 6202-6215
