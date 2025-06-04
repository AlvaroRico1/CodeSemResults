format_create(struct client *c, struct cmdq_item *item, int tag, int flags)
{
	struct format_tree	*ft;

	ft = xcalloc(1, sizeof *ft);
	RB_INIT(&ft->tree);

	if (c != NULL) {
		ft->client = c;
		ft->client->references++;
	}
	ft->item = item;

	ft->tag = tag;
	ft->flags = flags;

	if (item != NULL)
		format_create_add_item(ft, item);

	return (ft);
}


// Source: format.c
// Lines 3058-3078
