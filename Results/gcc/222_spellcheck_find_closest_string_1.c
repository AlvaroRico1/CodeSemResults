find_closest_string (const char *target,
		     const auto_vec<const char *> *candidates)
{
  gcc_assert (target);
  gcc_assert (candidates);

  int i;
  const char *candidate;
  best_match<const char *, const char *> bm (target);
  FOR_EACH_VEC_ELT (*candidates, i, candidate)
    {
      gcc_assert (candidate);
      bm.consider (candidate);
    }

  return bm.get_best_meaningful_candidate ();
}


// Source: spellcheck.c
// Lines 147-163
