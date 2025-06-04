static int nfs3_write_done(struct rpc_task *task, struct nfs_pgio_header *hdr)
{
	struct inode *inode = hdr->inode;

	if (hdr->pgio_done_cb != NULL)
		return hdr->pgio_done_cb(task, hdr);

	if (nfs3_async_handle_jukebox(task, inode))
		return -EAGAIN;
	if (task->tk_status >= 0)
		nfs_writeback_update_inode(hdr);
	return 0;
}


// Source: nfs3proc.c
// Lines 819-831
