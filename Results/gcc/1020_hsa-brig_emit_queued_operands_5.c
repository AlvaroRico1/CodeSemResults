emit_queued_operands (void)
{
  for (hsa_op_base *op = op_queue.first_op; op; op = op->m_next)
    {
      gcc_assert (op->m_brig_op_offset == brig_operand.total_size);
      if (hsa_op_immed *imm = dyn_cast <hsa_op_immed *> (op))
	emit_immediate_operand (imm);
      else if (hsa_op_reg *reg = dyn_cast <hsa_op_reg *> (op))
	emit_register_operand (reg);
      else if (hsa_op_address *addr = dyn_cast <hsa_op_address *> (op))
	emit_address_operand (addr);
      else if (hsa_op_code_ref *ref = dyn_cast <hsa_op_code_ref *> (op))
	emit_code_ref_operand (ref);
      else if (hsa_op_code_list *code_list = dyn_cast <hsa_op_code_list *> (op))
	emit_code_list_operand (code_list);
      else if (hsa_op_operand_list *l = dyn_cast <hsa_op_operand_list *> (op))
	emit_operand_list_operand (l);
      else
	gcc_unreachable ();
    }
}


// Source: hsa-brig.c
// Lines 1187-1207
