static void options_cmp(void *one, void *two, size_t size, const char *name)
{
	size_t i;

	for (i = 0; i < size; i++) {
		if (((char *)one)[i] != ((char *)two)[i]) {
			char desc[1024];

			p_snprintf(desc, 1024, "Difference in %s at byte %" PRIuZ ": macro=%u / func=%u",
				name, i, ((char *)one)[i], ((char *)two)[i]);
			clar__fail(__FILE__, __func__, __LINE__,
				"Difference between macro and function options initializer",
				desc, 0);
			return;
		}
	}
}


// Source: structinit.c
// Lines 42-58
