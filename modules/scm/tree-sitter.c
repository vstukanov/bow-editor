#include "libguile/alist.h"
#include "libguile/foreign.h"
#include "libguile/hashtab.h"
#include "libguile/list.h"
#include "libguile/modules.h"
#include "libguile/numbers.h"
#include "libguile/pairs.h"
#include "libguile/strings.h"
#include "libguile/symbols.h"
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

SCM
api_tree_cursor_current_node_to_alist(SCM _cursor)
{
  TSTreeCursor *cursor = (TSTreeCursor *) scm_to_pointer(_cursor);
  TSNode current_node = ts_tree_cursor_current_node(cursor);

  TSPoint start_point = ts_node_start_point(current_node);
  TSPoint end_point = ts_node_end_point(current_node);

  SCM start_list = scm_list_2(scm_from_uint32(start_point.row),
			      scm_from_uint32(start_point.column));

  SCM end_list = scm_list_2(scm_from_uint32(end_point.row),
			    scm_from_uint32(end_point.column));

  SCM node_type = scm_from_utf8_string(ts_node_type(current_node));

  return scm_list_3(node_type, start_list, end_list);
}

void
init_tree_sitter_cursor (void *unused)
{
  scm_c_define_gsubr("goto-first-child", 1, 0, 0, &api_tree_cursor_goto_first_child);
  scm_c_define_gsubr("goto-next-sibling", 1, 0, 0, &api_tree_cursor_goto_next_sibling);
  scm_c_define_gsubr("goto-parent", 1, 0, 0, &api_tree_cursor_goto_parent);
  scm_c_define_gsubr("current-node-type", 1, 0, 0, &api_tree_cursor_node_type);
  scm_c_define_gsubr("current-node->alist", 1, 0, 0, &api_tree_cursor_current_node_to_alist);

  scm_c_export("goto-first-child",
	       "goto-next-sibling",
	       "goto-parent",
	       "current-node-type",
	       "current-node->alist",
	       NULL);
}

void
scm_init_tree_sitter_module()
{
  scm_c_define_module("bow tree-sitter cursor", &init_tree_sitter_cursor, NULL);
}
