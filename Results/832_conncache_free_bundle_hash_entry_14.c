// Source: curl/lib/conncache.c
// Lines 107-109
static void free_bundle_hash_entry(void *freethis)
{
  struct connectbundle *b = (struct connectbundle *) freethis;

  bundle_destroy(b);
}
