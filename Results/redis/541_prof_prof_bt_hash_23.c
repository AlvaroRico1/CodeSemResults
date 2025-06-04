prof_bt_hash(const void *key, size_t r_hash[2]) {
	prof_bt_t *bt = (prof_bt_t *)key;

	cassert(config_prof);

	hash(bt->vec, bt->len * sizeof(void *), 0x94122f33U, r_hash);
}


// Source: prof.c
// Lines 1869-1875
