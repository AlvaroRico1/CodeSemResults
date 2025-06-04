static bool nfs_revoke_delegation(struct inode *inode,
		const nfs4_stateid *stateid)
{
	struct nfs_delegation *delegation;
	nfs4_stateid tmp;
	bool ret = false;

	rcu_read_lock();
	delegation = rcu_dereference(NFS_I(inode)->delegation);
	if (delegation == NULL)
		goto out;
	if (stateid == NULL) {
		nfs4_stateid_copy(&tmp, &delegation->stateid);
		stateid = &tmp;
	} else if (!nfs4_stateid_match(stateid, &delegation->stateid))
		goto out;
	nfs_mark_delegation_revoked(NFS_SERVER(inode), delegation);
	ret = true;
out:
	rcu_read_unlock();
	if (ret)
		nfs_inode_find_state_and_recover(inode, stateid);
	return ret;
}


// Source: delegation.c
// Lines 740-763
