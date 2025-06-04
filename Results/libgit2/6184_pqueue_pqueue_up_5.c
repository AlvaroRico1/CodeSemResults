static void pqueue_up(git_pqueue *pq, size_t el)
{
	size_t parent_el = PQUEUE_PARENT_OF(el);
	void *kid = git_vector_get(pq, el);

	while (el > 0) {
		void *parent = pq->contents[parent_el];

		if (pq->_cmp(parent, kid) <= 0)
			break;

		pq->contents[el] = parent;

		el = parent_el;
		parent_el = PQUEUE_PARENT_OF(el);
	}


// Source: pqueue.c
// Lines 36-51
