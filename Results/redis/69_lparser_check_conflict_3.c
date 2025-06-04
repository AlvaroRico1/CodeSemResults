static void check_conflict (LexState *ls, struct LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  int extra = fs->freereg;  /* eventual position to save local variable */
  int conflict = 0;
  for (; lh; lh = lh->prev) {
    if (lh->v.k == VINDEXED) {
      if (lh->v.u.s.info == v->u.s.info) {  /* conflict? */
        conflict = 1;
        lh->v.u.s.info = extra;  /* previous assignment will use safe copy */
      }
      if (lh->v.u.s.aux == v->u.s.info) {  /* conflict? */
        conflict = 1;
        lh->v.u.s.aux = extra;  /* previous assignment will use safe copy */
      }
    }
  }
  if (conflict) {
    luaK_codeABC(fs, OP_MOVE, fs->freereg, v->u.s.info, 0);  /* make copy */
    luaK_reserveregs(fs, 1);
  }
}


// Source: lparser.c
// Lines 908-928
