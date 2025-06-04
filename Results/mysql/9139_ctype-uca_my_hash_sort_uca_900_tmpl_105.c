static void my_hash_sort_uca_900_tmpl(const CHARSET_INFO *cs, const Mb_wc mb_wc,
                                      const uchar *s, size_t slen, uint64 *n1) {
  uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE> scanner(mb_wc, cs, s, slen);

  /*
    A variation of the FNV-1a hash. The differences between this and
    standard FNV-1a as described in literature are:

     - We work naturally on 16-bit weights, so we XOR in the entire weight
       instead of hashing byte-by-byte. (This is effectively a speed/quality
       tradeoff, as it will reduce avalanche.)
     - We use the n1 seed by XOR-ing it onto the offset basis; FNV-1a as
       typically described does not use a seed. This should be safe, since
       there's nothing magical about the offset basis; it's just the FNV-1a
       hash of some human-readable text.

    This is nowhere near a perfect hash function; it has suboptimal avalanche
    characteristics, and it not multicollision resistant. In particular,
    it fails many SMHasher tests, mostly for bias (collision tests are fine).
    However, it is of much better quality than the home-grown hash used
    for other collations (which fails _all_ SMHasher tests), while being
    much faster.

    We ignore the n2 seed entirely, since we don't need it. The caller is
    responsible for doing hash folding at the end; we can't do that.

    See http://isthe.com/chongo/tech/comp/fnv/#FNV-param for constants.
  */

  uint64 h = *n1;
  h ^= 14695981039346656037ULL;

  scanner.for_each_weight(
      [&](int s_res, bool) -> bool {
        h ^= s_res;
        h *= 1099511628211ULL;
        return true;
      },
      [](int) { return true; });

  *n1 = h;
}


// Source: ctype-uca.cc
// Lines 4937-4978
