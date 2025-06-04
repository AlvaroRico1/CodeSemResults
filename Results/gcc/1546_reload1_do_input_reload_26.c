do_input_reload (class insn_chain *chain, struct reload *rl, int j)
{
  rtx_insn *insn = chain->insn;
  rtx old = (rl->in && MEM_P (rl->in)
	     ? rl->in_reg : rl->in);
  rtx reg_rtx = rl->reg_rtx;

  if (old && reg_rtx)
    {
      machine_mode mode;

      /* Determine the mode to reload in.
	 This is very tricky because we have three to choose from.
	 There is the mode the insn operand wants (rl->inmode).
	 There is the mode of the reload register RELOADREG.
	 There is the intrinsic mode of the operand, which we could find
	 by stripping some SUBREGs.
	 It turns out that RELOADREG's mode is irrelevant:
	 we can change that arbitrarily.

	 Consider (SUBREG:SI foo:QI) as an operand that must be SImode;
	 then the reload reg may not support QImode moves, so use SImode.
	 If foo is in memory due to spilling a pseudo reg, this is safe,
	 because the QImode value is in the least significant part of a
	 slot big enough for a SImode.  If foo is some other sort of
	 memory reference, then it is impossible to reload this case,
	 so previous passes had better make sure this never happens.

	 Then consider a one-word union which has SImode and one of its
	 members is a float, being fetched as (SUBREG:SF union:SI).
	 We must fetch that as SFmode because we could be loading into
	 a float-only register.  In this case OLD's mode is correct.

	 Consider an immediate integer: it has VOIDmode.  Here we need
	 to get a mode from something else.

	 In some cases, there is a fourth mode, the operand's
	 containing mode.  If the insn specifies a containing mode for
	 this operand, it overrides all others.

	 I am not sure whether the algorithm here is always right,
	 but it does the right things in those cases.  */

      mode = GET_MODE (old);
      if (mode == VOIDmode)
	mode = rl->inmode;

      /* We cannot use gen_lowpart_common since it can do the wrong thing
	 when REG_RTX has a multi-word mode.  Note that REG_RTX must
	 always be a REG here.  */
      if (GET_MODE (reg_rtx) != mode)
	reg_rtx = reload_adjust_reg_for_mode (reg_rtx, mode);
    }
  reload_reg_rtx_for_input[j] = reg_rtx;

  if (old != 0
      /* AUTO_INC reloads need to be handled even if inherited.  We got an
	 AUTO_INC reload if reload_out is set but reload_out_reg isn't.  */
      && (! reload_inherited[j] || (rl->out && ! rl->out_reg))
      && ! rtx_equal_p (reg_rtx, old)
      && reg_rtx != 0)
    emit_input_reload_insns (chain, rld + j, old, j);

  /* When inheriting a wider reload, we have a MEM in rl->in,
     e.g. inheriting a SImode output reload for
     (mem:HI (plus:SI (reg:SI 14 fp) (const_int 10)))  */
  if (optimize && reload_inherited[j] && rl->in
      && MEM_P (rl->in)
      && MEM_P (rl->in_reg)
      && reload_spill_index[j] >= 0
      && TEST_HARD_REG_BIT (reg_reloaded_valid, reload_spill_index[j]))
    rl->in = regno_reg_rtx[reg_reloaded_contents[reload_spill_index[j]]];

  /* If we are reloading a register that was recently stored in with an
     output-reload, see if we can prove there was
     actually no need to store the old value in it.  */

  if (optimize
      && (reload_inherited[j] || reload_override_in[j])
      && reg_rtx
      && REG_P (reg_rtx)
      && spill_reg_store[REGNO (reg_rtx)] != 0
#if 0
      /* There doesn't seem to be any reason to restrict this to pseudos
	 and doing so loses in the case where we are copying from a
	 register of the wrong class.  */
      && !HARD_REGISTER_P (spill_reg_stored_to[REGNO (reg_rtx)])
#endif
      /* The insn might have already some references to stackslots
	 replaced by MEMs, while reload_out_reg still names the
	 original pseudo.  */
      && (dead_or_set_p (insn, spill_reg_stored_to[REGNO (reg_rtx)])
	  || rtx_equal_p (spill_reg_stored_to[REGNO (reg_rtx)], rl->out_reg)))
    delete_output_reload (insn, j, REGNO (reg_rtx), reg_rtx);
}


// Source: reload1.c
// Lines 7757-7851
