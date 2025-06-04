prof_bt_keycomp(const void *k1, const void *k2) {
	const prof_bt_t *bt1 = (prof_bt_t *)k1;
	const prof_bt_t *bt2 = (prof_bt_t *)k2;

	cassert(config_prof);

	if (bt1->len != bt2->len) {
		return false;
	}
	return (memcmp(bt1->vec, bt2->vec, bt1->len * sizeof(void *)) == 0);
}


// Source: prof.c
// Lines 1878-1888
