static void pqueue_down(git_pqueue *pq, size_t el)
{
	void *parent = git_vector_get(pq, el), *kid, *rkid;

	while (1) {
		size_t kid_el = PQUEUE_LCHILD_OF(el);

		if ((kid = git_vector_get(pq, kid_el)) == NULL)
			break;

		if ((rkid = git_vector_get(pq, kid_el + 1)) != NULL &&
			pq->_cmp(kid, rkid) > 0) {
			kid    = rkid;
			kid_el += 1;
		}

		if (pq->_cmp(parent, kid) <= 0)
			break;

		pq->contents[el] = kid;
		el = kid_el;
	}

	pq->contents[el] = parent;
}


// Source: pqueue.c
// Lines 56-80
