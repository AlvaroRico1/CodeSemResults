pgcmp (const void *v1_, const void *v2_)
{
  const vertex *v1 = (const vertex *)v1_;
  const vertex *v2 = (const vertex *)v2_;
  return v2->post - v1->post;
}


// Source: tree-loop-distribution.c
// Lines 2114-2119
