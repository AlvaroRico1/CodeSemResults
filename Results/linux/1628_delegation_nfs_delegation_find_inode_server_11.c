nfs_delegation_find_inode_server(struct nfs_server *server,
				 const struct nfs_fh *fhandle)
{
	struct nfs_delegation *delegation;
	struct inode *freeme, *res = NULL;

	list_for_each_entry_rcu(delegation, &server->delegations, super_list) {
		spin_lock(&delegation->lock);
		if (delegation->inode != NULL &&
		    nfs_compare_fh(fhandle, &NFS_I(delegation->inode)->fh) == 0) {
			freeme = igrab(delegation->inode);
			if (freeme && nfs_sb_active(freeme->i_sb))
				res = freeme;
			spin_unlock(&delegation->lock);
			if (res != NULL)
				return res;
			if (freeme) {
				rcu_read_unlock();
				iput(freeme);
				rcu_read_lock();
			}
			return ERR_PTR(-EAGAIN);
		}
		spin_unlock(&delegation->lock);
	}
	return ERR_PTR(-ENOENT);
}


// Source: delegation.c
// Lines 850-876
