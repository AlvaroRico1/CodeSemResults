static double ratio(Bigint *a, Bigint *b) {
  U da, db;
  int k, ka, kb;

  dval(&da) = b2d(a, &ka);
  dval(&db) = b2d(b, &kb);
  k = ka - kb + 32 * (a->wds - b->wds);
  if (k > 0)
    word0(&da) += k * Exp_msk1;
  else {
    k = -k;
    word0(&db) += k * Exp_msk1;
  }
  return dval(&da) / dval(&db);
}


// Source: dtoa.cc
// Lines 1209-1223
