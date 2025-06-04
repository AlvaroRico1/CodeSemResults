static struct nfs_netns_client *nfs_netns_client_alloc(struct kobject *parent,
		struct net *net)
{
	struct nfs_netns_client *p;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (p) {
		p->net = net;
		p->kobject.kset = nfs_client_kset;
		if (kobject_init_and_add(&p->kobject, &nfs_netns_client_type,
					parent, "nfs_client") == 0)
			return p;
		kobject_put(&p->kobject);
	}
	return NULL;
}


// Source: sysfs.c
// Lines 149-164
