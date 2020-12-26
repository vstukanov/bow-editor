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
private:
  Gtk::TextView m_pTextView;
  Gtk::ScrolledWindow m_pScrolledWindow;
};

MyWindow::MyWindow() : m_pTextView()
{
  set_title("Bow — text editor");
  set_default_size(400, 400);

  m_pTextView.set_size_request(300, 300);
  
  m_pScrolledWindow.add(m_pTextView);
  add(m_pScrolledWindow);

  show_all();
}

MyWindow::~MyWindow() {}

// buffer api call
void MyWindow::open(const std::string &filename) {
  std::string fullpath = Glib::canonicalize_filename(filename, Glib::get_current_dir());

  std::cout << "file_get_content(" << fullpath << ")" << std::endl;
  std::string content = Glib::file_get_contents(fullpath);

  auto refBuffer = m_pTextView.get_buffer().get();
  refBuffer->set_text(content);

  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_javascript());

  TSTree *tree = ts_parser_parse_string(parser, NULL, content.c_str(), content.length());

  TSNode root_node = ts_tree_root_node(tree);
  std::cout << ts_node_string(root_node) << std::endl;
}

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create("me.oddy.bow");
  MyWindow window;

  // window.open("sample/react.development.js");
  window.open("../sample/simple.js");
  
  app->run(window, argc, argv);
}
