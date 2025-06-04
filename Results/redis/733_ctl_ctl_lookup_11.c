ctl_lookup(tsdn_t *tsdn, const char *name, ctl_node_t const **nodesp,
    size_t *mibp, size_t *depthp) {
	int ret;
	const char *elm, *tdot, *dot;
	size_t elen, i, j;
	const ctl_named_node_t *node;

	elm = name;
	/* Equivalent to strchrnul(). */
	dot = ((tdot = strchr(elm, '.')) != NULL) ? tdot : strchr(elm, '\0');
	elen = (size_t)((uintptr_t)dot - (uintptr_t)elm);
	if (elen == 0) {
		ret = ENOENT;
		goto label_return;
	}
	node = super_root_node;
	for (i = 0; i < *depthp; i++) {
		assert(node);
		assert(node->nchildren > 0);
		if (ctl_named_node(node->children) != NULL) {
			const ctl_named_node_t *pnode = node;

			/* Children are named. */
			for (j = 0; j < node->nchildren; j++) {
				const ctl_named_node_t *child =
				    ctl_named_children(node, j);
				if (strlen(child->name) == elen &&
				    strncmp(elm, child->name, elen) == 0) {
					node = child;
					if (nodesp != NULL) {
						nodesp[i] =
						    (const ctl_node_t *)node;
					}
					mibp[i] = j;
					break;
				}
			}


// Source: ctl.c
// Lines 1075-1111
