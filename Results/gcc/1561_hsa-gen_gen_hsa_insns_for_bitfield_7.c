gen_hsa_insns_for_bitfield (hsa_op_reg *dest, hsa_op_reg *value_reg,
			    HOST_WIDE_INT bitsize, HOST_WIDE_INT bitpos,
			    hsa_bb *hbb)
{
  unsigned type_bitsize
    = hsa_type_bit_size (hsa_extend_inttype_to_32bit (dest->m_type));
  unsigned left_shift = type_bitsize - (bitsize + bitpos);
  unsigned right_shift = left_shift + bitpos;

  if (left_shift)
    {
      hsa_op_reg *value_reg_2
	= new hsa_op_reg (hsa_extend_inttype_to_32bit (dest->m_type));
      hsa_op_immed *c = new hsa_op_immed (left_shift, BRIG_TYPE_U32);

      hsa_insn_basic *lshift
	= new hsa_insn_basic (3, BRIG_OPCODE_SHL, value_reg_2->m_type,
			      value_reg_2, value_reg, c);

      hbb->append_insn (lshift);

      value_reg = value_reg_2;
    }


// Source: hsa-gen.c
// Lines 2327-2349
