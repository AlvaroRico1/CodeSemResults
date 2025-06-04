assign_dfs_numbers (struct et_node *node, int *num)
{
  struct et_node *son;

  node->dfs_num_in = (*num)++;

  if (node->son)
    {
      assign_dfs_numbers (node->son, num);
      for (son = node->son->right; son != node->son; son = son->right)
	assign_dfs_numbers (son, num);
    }

  node->dfs_num_out = (*num)++;
}


// Source: dominance.c
// Lines 640-654
