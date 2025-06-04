cmdq_append(struct client *c, struct cmdq_item *item)
{
	struct cmdq_list	*queue = cmdq_get(c);
	struct cmdq_item	*next;

	do {
		next = item->next;
		item->next = NULL;

		if (c != NULL)
			c->references++;
		item->client = c;

		item->queue = queue;
		TAILQ_INSERT_TAIL(&queue->list, item, entry);
		log_debug("%s %s: %s", __func__, cmdq_name(c), item->name);

		item = next;
	} while (item != NULL);
	return (TAILQ_LAST(&queue->list, cmdq_item_list));
}


// Source: cmd-queue.c
// Lines 297-317
