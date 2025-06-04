static int buf_to_pages_noslab(const void *buf, size_t buflen,
		struct page **pages)
{
	struct page *newpage, **spages;
	int rc = 0;
	size_t len;
	spages = pages;

	do {
		len = min_t(size_t, PAGE_SIZE, buflen);
		newpage = alloc_page(GFP_KERNEL);

		if (newpage == NULL)
			goto unwind;
		memcpy(page_address(newpage), buf, len);
		buf += len;
		buflen -= len;
		*pages++ = newpage;
		rc++;
	} while (buflen != 0);

	return rc;

unwind:
	for(; rc > 0; rc--)
		__free_page(spages[rc-1]);
	return -ENOMEM;
}


// Source: nfs4proc.c
// Lines 5417-5444
