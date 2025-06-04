sort_by_decl_uid (const void *a, const void *b)
{
  const tree *t1 = (const tree *)a;
  const tree *t2 = (const tree *)b;

  int uid1 = DECL_UID (*t1);
  int uid2 = DECL_UID (*t2);

  if (uid1 < uid2)
    return -1;
  else if (uid1 > uid2)
    return 1;
  else
    return 0;
}


// Source: gimplify.c
// Lines 1279-1293
