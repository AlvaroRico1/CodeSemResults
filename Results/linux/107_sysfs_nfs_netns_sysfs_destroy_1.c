void nfs_netns_sysfs_destroy(struct nfs_net *netns)
{
	struct nfs_netns_client *clp = netns->nfs_client;

	if (clp) {
		kobject_uevent(&clp->kobject, KOBJ_REMOVE);
		kobject_del(&clp->kobject);
		kobject_put(&clp->kobject);
		netns->nfs_client = NULL;
	}
}


// Source: sysfs.c
// Lines 177-187
