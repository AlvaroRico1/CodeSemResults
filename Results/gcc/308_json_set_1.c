object::set (const char *key, value *v)
{
  gcc_assert (key);
  gcc_assert (v);

  value **ptr = m_map.get (key);
  if (ptr)
    {
      /* If the key is already present, delete the existing value
	 and overwrite it.  */
      delete *ptr;
      *ptr = v;
    }
  else
    /* If the key wasn't already present, take a copy of the key,
       and store the value.  */
    m_map.put (xstrdup (key), v);
}


// Source: json.cc
// Lines 83-100
