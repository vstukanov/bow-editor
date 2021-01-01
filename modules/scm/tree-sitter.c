#include "libguile/foreign.h"
#include <tree_sitter/api.h>
#include <libguile.h>

SCM
api_tree_cursor_goto_first_child(SCM _cursor)
{
  TSTreeCursor *cursor = (TSTreeCursor *) scm_to_pointer(_cursor);
  return scm_from_bool(ts_tree_cursor_goto_first_child(cursor));
}

SCM
api_tree_cursor_goto_next_sibling(SCM _cursor)
{
  TSTreeCursor *cursor = (TSTreeCursor *) scm_to_pointer(_cursor);
  return scm_from_bool(ts_tree_cursor_goto_next_sibling(cursor));
}

SCM
api_tree_cursor_goto_parent(SCM _cursor)
{
  TSTreeCursor *cursor = (TSTreeCursor *) scm_to_pointer(_cursor);
  return scm_from_bool(ts_tree_cursor_goto_parent(cursor));
}

SCM
api_tree_cursor_node_type(SCM _cursor)
{
  TSTreeCursor *cursor = (TSTreeCursor *) scm_to_pointer(_cursor);
  TSNode current_node = ts_tree_cursor_current_node(cursor);

  return scm_from_utf8_string(ts_node_type(current_node));
}

void
init_tree_sitter (void *unused)
{
  scm_c_define_gsubr("goto-first-child", 1, 0, 0, &api_tree_cursor_goto_first_child);
  scm_c_define_gsubr("goto-next-sibling", 1, 0, 0, &api_tree_cursor_goto_next_sibling);
  scm_c_define_gsubr("goto-parent", 1, 0, 0, &api_tree_cursor_goto_parent);
  scm_c_define_gsubr("current-node-type", 1, 0, 0, &api_tree_cursor_node_type);

  scm_c_export("goto-first-child",
	       "goto-next-sibling",
	       "goto-parent",
	       "current-node-type",
	       NULL);
}

void
scm_init_tree_sitter_module()
{
  scm_c_define_module("bow tree-sitter cursor", &init_tree_sitter, NULL);
}
