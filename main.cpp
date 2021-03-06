#include "gtkmm/textbuffer.h"
#include "libguile/scm.h"
#include <cstddef>
#include <cstdint>
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
#include <stdint.h>
#include <string>
#include <libguile.h>

#include <tree_sitter/api.h>

#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

const std::string BASE = "/home/vst/Dev/snaex/bow-editor/";
SCM scm_init_module;

// implemented by `tree-sitter-javascript` library.
extern "C" TSLanguage *tree_sitter_javascript();

class MyWindow : public Gtk::Window 
{
public:
  MyWindow();
  virtual ~MyWindow();

  void open(const std::string &path);
  void set_font(const std::string &font, uint font_size);
  std::string get_text(uint32_t row, uint32_t column);
  Gtk::TextView m_pTextView;
private:
  std::set<std::string> m_supported_tags;
  Glib::RefPtr<Gtk::TextTag> create_theme_tag(const std::string &name, const std::string &fg_color);
  void create_theme();
  void highlight_node(TSTreeCursor *node, int level);
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

  SCM parceProc = scm_variable_ref(scm_c_lookup("parse-tree"));
  SCM result = scm_call_1(parceProc, scm_from_pointer(&cursor, NULL));
}

MyWindow *window = nullptr;

Gtk::TextBuffer::iterator scm_list_to_iter(Glib::RefPtr<Gtk::TextBuffer> buffer, SCM list) {
  uint32_t row = scm_to_uint32(scm_list_ref(list, scm_from_uint(0)));
  uint32_t column = scm_to_uint32(scm_list_ref(list, scm_from_uint(1)));
  return buffer->get_iter_at_line_offset(row, column);
}

extern "C" {
  static SCM api_find_file(SCM scm_path) {
    size_t path_length;
    char *cpath = scm_to_utf8_stringn(scm_path, &path_length);

    window->open(std::string(cpath, path_length));

    return SCM_UNSPECIFIED;
  }

  static SCM api_get_text(SCM start, SCM end) {
    auto buffer = window->m_pTextView.get_buffer();
    
    auto text = buffer->get_text(scm_list_to_iter(buffer, start),
				 scm_list_to_iter(buffer, end));

    return scm_from_utf8_string(text.c_str());
  }

  static SCM api_apply_tag(SCM tag, SCM start, SCM end) {
    auto buffer = window->m_pTextView.get_buffer();
    std::string tagname(scm_to_utf8_string(tag));

    // std::cout << " [" << tagname << "] " << std::endl;
    buffer->apply_tag_by_name(tagname,
			      scm_list_to_iter(buffer, start),
			      scm_list_to_iter(buffer, end));

    return SCM_UNDEFINED;
  }

  static void scm_main(void *closure, int argc, char *argv[]) {
    auto app = Gtk::Application::create("me.oddy.bow");
    window = new MyWindow();

    // window.open("sample/react.development.js");
    window->set_font("DejaVu Sans Mono", 10);

    scm_c_load_extension("./libscm_tree_sitter.so", "scm_init_tree_sitter_module");

    scm_c_define("editor-base", scm_from_utf8_string(BASE.c_str()));
    scm_c_define_gsubr("find-file", 1, 0, 0, (scm_t_subr) &api_find_file);
    scm_c_define_gsubr("buffer-get-text", 2, 0, 0, (scm_t_subr) &api_get_text);
    scm_c_define_gsubr("buffer-apply-tag", 3, 0, 0, (scm_t_subr) &api_apply_tag);
    
    scm_init_module = scm_c_primitive_load((BASE + "scripts/init.scm").c_str());

    SCM proc = scm_variable_ref(scm_c_lookup("main"));
    scm_call_0(proc);

    app->run(*window, argc, argv);
  }
}

int main(int argc, char* argv[]) {
  scm_boot_guile(argc, argv, scm_main, 0);
  return 0;
}
