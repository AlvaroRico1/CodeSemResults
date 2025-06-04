PREFIX(store_op1) (re_opcode_t op, UCHAR_T *loc, int arg)
{
  *loc = (UCHAR_T) op;
  STORE_NUMBER (loc + 1, arg);
}


// Source: regex.c
// Lines 4231-4235
