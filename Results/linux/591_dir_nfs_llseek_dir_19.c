static loff_t nfs_llseek_dir(struct file *filp, loff_t offset, int whence)
{
	struct inode *inode = file_inode(filp);
	struct nfs_open_dir_context *dir_ctx = filp->private_data;

	dfprintk(FILE, "NFS: llseek dir(%pD2, %lld, %d)\n",
			filp, offset, whence);

	switch (whence) {
	default:
		return -EINVAL;
	case SEEK_SET:
		if (offset < 0)
			return -EINVAL;
		inode_lock(inode);
		break;
	case SEEK_CUR:
		if (offset == 0)
			return filp->f_pos;
		inode_lock(inode);
		offset += filp->f_pos;
		if (offset < 0) {
			inode_unlock(inode);
			return -EINVAL;
		}
	}
	if (offset != filp->f_pos) {
		filp->f_pos = offset;
		dir_ctx->dir_cookie = 0;
		dir_ctx->duped = 0;
	}
	inode_unlock(inode);
	return offset;
}


// Source: dir.c
// Lines 900-933
