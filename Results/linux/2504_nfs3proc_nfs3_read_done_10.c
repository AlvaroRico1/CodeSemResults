static int nfs3_read_done(struct rpc_task *task, struct nfs_pgio_header *hdr)
{
	struct inode *inode = hdr->inode;
	struct nfs_server *server = NFS_SERVER(inode);

	if (hdr->pgio_done_cb != NULL)
		return hdr->pgio_done_cb(task, hdr);

	if (nfs3_async_handle_jukebox(task, inode))
		return -EAGAIN;

	if (task->tk_status >= 0 && !server->read_hdrsize)
		cmpxchg(&server->read_hdrsize, 0, hdr->res.replen);

	nfs_invalidate_atime(inode);
	nfs_refresh_inode(inode, &hdr->fattr);
	return 0;
}


// Source: nfs3proc.c
// Lines 786-803
