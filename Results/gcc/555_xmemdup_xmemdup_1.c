xmemdup (const PTR input, size_t copy_size, size_t alloc_size)
{
  PTR output = xmalloc (alloc_size);
  if (alloc_size > copy_size)
    memset ((char *) output + copy_size, 0, alloc_size - copy_size);
  return (PTR) memcpy (output, input, copy_size);
}


// Source: xmemdup.c
// Lines 35-41
