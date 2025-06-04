void git_vector_reverse(git_vector *v)
{
	size_t a, b;

	if (v->length == 0)
		return;

	a = 0;
	b = v->length - 1;

	while (a < b) {
		void *tmp = v->contents[a];
		v->contents[a] = v->contents[b];
		v->contents[b] = tmp;
		a++;
		b--;
	}


// Source: vector.c
// Lines 414-430
