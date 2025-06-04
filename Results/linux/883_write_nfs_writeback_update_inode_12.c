void nfs_writeback_update_inode(struct nfs_pgio_header *hdr)
{
	struct nfs_fattr *fattr = &hdr->fattr;
	struct inode *inode = hdr->inode;

	spin_lock(&inode->i_lock);
	nfs_writeback_check_extend(hdr, fattr);
	nfs_post_op_update_inode_force_wcc_locked(inode, fattr);
	spin_unlock(&inode->i_lock);
}


// Source: write.c
// Lines 1537-1546
