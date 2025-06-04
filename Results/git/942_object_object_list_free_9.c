void object_list_free(struct object_list **list)
{
	while (*list) {
		struct object_list *p = *list;
		*list = p->next;
		free(p);
	}


// Source: object.c
// Lines 328-334
