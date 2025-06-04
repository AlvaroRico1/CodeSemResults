cmdq_remove_group(struct cmdq_item *item)
{
	struct cmdq_item	*this, *next;

	if (item->group == 0)
		return;
	this = TAILQ_NEXT(item, entry);
	while (this != NULL) {
		next = TAILQ_NEXT(this, entry);
		if (this->group == item->group)
			cmdq_remove(this);
		this = next;
	}
}


// Source: cmd-queue.c
// Lines 466-479
