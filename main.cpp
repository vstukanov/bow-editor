#include <cstddef>
#include <fstream>
#include <giomm/application.h>
#include <glibmm/miscutils.h>
#include <gtkmm/textiter.h>
#include <iterator>
#include <pangomm/fontdescription.h>
#include <cstdlib>
#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <libguile.h>

#include <tree_sitter/api.h>

#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

const std::string BASE = "/home/vst/Dev/snaex/bow-editor/";

// implemented by `tree-sitter-javascript` library.
extern "C" TSLanguage *tree_sitter_javascript();

class MyWindow : public Gtk::Window 
{
public:
  MyWindow();
  virtual ~MyWindow();

  void open(const std::string &path);
  void set_font(const std::string &font, uint font_size);
private:
  std::set<std::string> m_supported_tags;
  Glib::RefPtr<Gtk::TextTag> create_theme_tag(const std::string &name, const std::string &fg_color);
  void create_theme();
  void highlight_node(TSTreeCursor *node, int level);
  Gtk::TextView m_pTextView;
  Gtk::ScrolledWindow m_pScrolledWindow;
};

void MyWindow::set_font(const std::string &font_family, uint font_size) {
  Pango::FontDescription fd;
  fd.set_family(font_family);
  fd.set_size(font_size * Pango::SCALE);

  m_pTextView.override_font(fd);
}

MyWindow::MyWindow() : m_pTextView(), m_supported_tags()
{
  set_title("Bow — text editor");
  set_default_size(400, 400);

  m_pTextView.set_size_request(300, 300);

  m_pScrolledWindow.add(m_pTextView);
  add(m_pScrolledWindow);

  create_theme();

  show_all();
}

MyWindow::~MyWindow() {}

Glib::RefPtr<Gtk::TextTag> MyWindow::create_theme_tag(const std::string &name, const std::string &fg_color) {
  auto tag = Gtk::TextTag::create(name);
  tag->property_foreground_rgba() = Gdk::RGBA(fg_color);
  m_supported_tags.insert(name);
  return tag;
}

void MyWindow::create_theme() {
  auto rTagTable = m_pTextView.get_buffer()->get_tag_table();

  auto commentTag = create_theme_tag("comment", "#8c8c8c");
  commentTag->property_style() = Pango::STYLE_ITALIC;
  rTagTable->add(commentTag);

  rTagTable->add(create_theme_tag("keyword", "#0033b3"));
  rTagTable->add(create_theme_tag("variable.builtin", "#0033b3"));
  rTagTable->add(create_theme_tag("function.builtin", "#0033b3"));
  rTagTable->add(create_theme_tag("constant.builtin", "#0033b3"));

  rTagTable->add(create_theme_tag("function.method", "#7a7a43"));
  
  rTagTable->add(create_theme_tag("number", "#1750eb"));
  rTagTable->add(create_theme_tag("string", "#067d17"));
  rTagTable->add(create_theme_tag("constructor", "#000"));
}

void MyWindow::highlight_node(TSTreeCursor *cursor, int level) {
  TSNode node = ts_tree_cursor_current_node(cursor);
  std::string node_type = ts_node_type(node);

  std::cout << std::string(level * 2, ' ') << "- " << node_type << std::endl;

  if (ts_tree_cursor_goto_first_child(cursor)) {
    highlight_node(cursor, level + 1);
    
    while(ts_tree_cursor_goto_next_sibling(cursor)) {
      highlight_node(cursor, level + 1);
    }
    
    ts_tree_cursor_goto_parent(cursor);
  }
}

// buffer api call
void MyWindow::open(const std::string &filename) {
  std::string fullpath = Glib::canonicalize_filename(filename, Glib::get_current_dir());

  std::cout << "file_get_content(" << fullpath << ")" << std::endl;

  if (!Glib::file_test(fullpath, Glib::FILE_TEST_EXISTS)) {
    std::cerr << "File " << fullpath << "doesn't exits." << std::endl;
    return;
  }

  std::string content;

  try {
    content = Glib::file_get_contents(fullpath);
  } catch (Glib::FileError fileError) {
    std::cerr << fileError.what() << std::endl;
  }

  auto refBuffer = m_pTextView.get_buffer();
  refBuffer->set_text(content);

  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_javascript());

  TSTree *tree = ts_parser_parse_string(parser, NULL, content.c_str(), content.length());

  TSNode root_node = ts_tree_root_node(tree);
  TSTreeCursor cursor = ts_tree_cursor_new(root_node);

  std::string queris = Glib::file_get_contents(BASE + "libs/tree-sitter-javascript/queries/highlights.scm");
  uint err_offset = 0;
  TSQueryError query_error;
  TSQuery* query = ts_query_new(tree_sitter_javascript(),
				queris.c_str(),
				queris.length(),
				&err_offset,
				&query_error);

  if (query_error != TSQueryErrorNone) {
    std::cout << "query error" << std::endl;
    return;
  }
  
  TSQueryCursor* qcursor = ts_query_cursor_new();
  TSQueryMatch qmatch;
  
  ts_query_cursor_exec(qcursor, query, root_node);

  while (ts_query_cursor_next_match(qcursor, &qmatch)) {
    for (int i = 0; i < qmatch.capture_count; ++i) {
      uint capture_length = 0;
      const char* capture_name_c = ts_query_capture_name_for_id(query,
								qmatch.captures[i].index,
								&capture_length);
      std::string capture_name(capture_name_c, capture_length);

      TSNode node = qmatch.captures[i].node;
      TSPoint node_start = ts_node_start_point(node);
      TSPoint node_end = ts_node_end_point(node);

      auto is = refBuffer->get_iter_at_line_offset(node_start.row, node_start.column);
      auto ie = refBuffer->get_iter_at_line_offset(node_end.row, node_end.column);

      if (std::find(m_supported_tags.cbegin(), m_supported_tags.cend(), capture_name) != m_supported_tags.cend()) {
	refBuffer->apply_tag_by_name(capture_name, is, ie);
      } else {
	std::cout << "! ";
      }

      printf("%s -- {%d, %d} - {%d, %d} (%s)\n", capture_name.c_str(), node_start.row, node_start.column, node_end.row, node_end.column, refBuffer->get_text(is, ie, false).c_str());
    }
    std::cout << std::endl;
  }
}

MyWindow *window = nullptr;

extern "C" {
  static SCM api_find_file(SCM scm_path) {
    size_t path_length;
    char *cpath = scm_to_utf8_stringn(scm_path, &path_length);

    window->open(std::string(cpath, path_length));

    return SCM_UNSPECIFIED;
  }
  
  static void scm_main(void *closure, int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;

    auto app = Gtk::Application::create("me.oddy.bow");
    window = new MyWindow();

    // window.open("sample/react.development.js");
    window->set_font("DejaVu Sans Mono", 10);

    scm_c_define("editor-base", scm_from_utf8_string(BASE.c_str()));
    scm_c_define_gsubr("find-file", 1, 0, 0, (scm_t_subr) &api_find_file);
    scm_c_primitive_load((BASE + "scripts/init.scm").c_str());
  
    app->run(*window, argc, argv);
  }
}

int main(int argc, char* argv[]) {
  scm_boot_guile(argc, argv, scm_main, 0);
  return 0;
}
