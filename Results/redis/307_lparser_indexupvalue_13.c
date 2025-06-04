static int indexupvalue (FuncState *fs, TString *name, expdesc *v) {
  int i;
  Proto *f = fs->f;
  int oldsize = f->sizeupvalues;
  for (i=0; i<f->nups; i++) {
    if (fs->upvalues[i].k == v->k && fs->upvalues[i].info == v->u.s.info) {
      lua_assert(f->upvalues[i] == name);
      return i;
    }
  }
  /* new one */
  luaY_checklimit(fs, f->nups + 1, LUAI_MAXUPVALUES, "upvalues");
  luaM_growvector(fs->L, f->upvalues, f->nups, f->sizeupvalues,
                  TString *, MAX_INT, "");
  while (oldsize < f->sizeupvalues) f->upvalues[oldsize++] = NULL;
  f->upvalues[f->nups] = name;
  luaC_objbarrier(fs->L, f, name);
  lua_assert(v->k == VLOCAL || v->k == VUPVAL);
  fs->upvalues[f->nups].k = cast_byte(v->k);
  fs->upvalues[f->nups].info = cast_byte(v->u.s.info);
  return f->nups++;
}


// Source: lparser.c
// Lines 183-204
