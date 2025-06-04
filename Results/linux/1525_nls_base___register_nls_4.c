int __register_nls(struct nls_table *nls, struct module *owner)
{
	struct nls_table ** tmp = &tables;

	if (nls->next)
		return -EBUSY;

	nls->owner = owner;
	spin_lock(&nls_lock);
	while (*tmp) {
		if (nls == *tmp) {
			spin_unlock(&nls_lock);
			return -EBUSY;
		}
		tmp = &(*tmp)->next;
	}
	nls->next = tables;
	tables = nls;
	spin_unlock(&nls_lock);
	return 0;	
}


// Source: nls_base.c
// Lines 235-255
