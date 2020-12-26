#include "giomm/application.h"
#include "glibmm/miscutils.h"
#include <cstdlib>
#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <string>

#include <tree_sitter/api.h>

#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

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
  Glib::RefPtr<Gtk::TextTag> create_theme_tag(const std::string &name, const std::string &fg_color);
  void create_theme();
  Gtk::TextView m_pTextView;
  Gtk::ScrolledWindow m_pScrolledWindow;
};

void MyWindow::set_font(const std::string &font_family, uint font_size) {
  Pango::FontDescription fd;
  fd.set_family(font_family);
  fd.set_size(font_size * Pango::SCALE);

  m_pTextView.override_font(fd);
}

MyWindow::MyWindow() : m_pTextView()
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
  return tag;
}

void MyWindow::create_theme() {
  auto rTagTable = m_pTextView.get_buffer()->get_tag_table();

  rTagTable->add(create_theme_tag("hl-comment", "#8c8c8c"));
  rTagTable->add(create_theme_tag("hl-keyword", "#3333BB"));
}

// buffer api call
void MyWindow::open(const std::string &filename) {
  std::string fullpath = Glib::canonicalize_filename(filename, Glib::get_current_dir());

  std::cout << "file_get_content(" << fullpath << ")" << std::endl;
  std::string content = Glib::file_get_contents(fullpath);

  auto refBuffer = m_pTextView.get_buffer();
  refBuffer->set_text(content);

  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_javascript());

  TSTree *tree = ts_parser_parse_string(parser, NULL, content.c_str(), content.length());

  TSNode root_node = ts_tree_root_node(tree);
  TSTreeCursor cursor = ts_tree_cursor_new(root_node);

  ts_tree_cursor_goto_first_child(&cursor);

  TSNode cn = ts_tree_cursor_current_node(&cursor);
  TSPoint sp = ts_node_start_point(cn);
  TSPoint ep = ts_node_end_point(cn);

  std::cout << ts_node_type(cn) << std::endl;
  refBuffer->apply_tag_by_name("hl-comment",
                               refBuffer->get_iter_at_line_offset(sp.row, sp.column),
                               refBuffer->get_iter_at_line_offset(ep.row, ep.column));

  std::cout << ts_node_string(root_node) << std::endl;
}

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create("me.oddy.bow");
  MyWindow window;

  // window.open("sample/react.development.js");
  window.open("/Users/vstukanov/Dev/gnome/bow-editor/sample/simple.js");
  window.set_font("DejaVu Sans Mono", 13);
  
  app->run(window, argc, argv);
}
