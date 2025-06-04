nfs_init_locked(struct inode *inode, void *opaque)
{
	struct nfs_find_desc	*desc = (struct nfs_find_desc *)opaque;
	struct nfs_fattr	*fattr = desc->fattr;

	set_nfs_fileid(inode, fattr->fileid);
	inode->i_mode = fattr->mode;
	nfs_copy_fh(NFS_FH(inode), desc->fh);
	return 0;
}


// Source: inode.c
// Lines 323-332
