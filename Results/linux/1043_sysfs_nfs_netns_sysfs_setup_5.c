void nfs_netns_sysfs_setup(struct nfs_net *netns, struct net *net)
{
	struct nfs_netns_client *clp;

	clp = nfs_netns_client_alloc(nfs_client_kobj, net);
	if (clp) {
		netns->nfs_client = clp;
		kobject_uevent(&clp->kobject, KOBJ_ADD);
	}
}


// Source: sysfs.c
// Lines 166-175
