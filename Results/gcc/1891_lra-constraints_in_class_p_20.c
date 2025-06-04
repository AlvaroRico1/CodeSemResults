in_class_p (rtx reg, enum reg_class cl, enum reg_class *new_class)
{
  enum reg_class rclass, common_class;
  machine_mode reg_mode;
  rtx src;
  int class_size, hard_regno, nregs, i, j;
  int regno = REGNO (reg);

  if (new_class != NULL)
    *new_class = NO_REGS;
  if (regno < FIRST_PSEUDO_REGISTER)
    {
      rtx final_reg = reg;
      rtx *final_loc = &final_reg;

      lra_eliminate_reg_if_possible (final_loc);
      return TEST_HARD_REG_BIT (reg_class_contents[cl], REGNO (*final_loc));
    }


// Source: lra-constraints.c
// Lines 243-260
