static void init_once(void *foo)
{
	struct nfs_inode *nfsi = (struct nfs_inode *) foo;

	inode_init_once(&nfsi->vfs_inode);
	INIT_LIST_HEAD(&nfsi->open_files);
	INIT_LIST_HEAD(&nfsi->access_cache_entry_lru);
	INIT_LIST_HEAD(&nfsi->access_cache_inode_lru);
	INIT_LIST_HEAD(&nfsi->commit_info.list);
	atomic_long_set(&nfsi->nrequests, 0);
	atomic_long_set(&nfsi->commit_info.ncommit, 0);
	atomic_set(&nfsi->commit_info.rpcs_out, 0);
	init_rwsem(&nfsi->rmdir_sem);
	mutex_init(&nfsi->commit_mutex);
	nfs4_init_once(nfsi);
}


// Source: inode.c
// Lines 2096-2111
