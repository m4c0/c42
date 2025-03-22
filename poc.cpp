#pragma leco tool
import c42;
import hai;
import jojo;
import jute;
import print;

int main() try {
  jute::view fn = "tests/embedding.cpp";
  auto buf = jojo::read_cstr(fn);
  auto tokens = c42::preprocess(buf);

  const auto log = [&](auto t, jute::view lvl) {
    errln(fn, ":", t.line, ":", t.column, ": [", lvl, "] ", t.value);
  };

  bool has_error = false;
  for (auto t : tokens) {
    if (t.type == c42::t_warning) {
      log(t, "warning");
    } else if (t.type == c42::t_error) {
      log(t, "error");
      has_error = true;
    }
  }
  if (has_error) return 1;

  for (auto t : tokens) {
    putf("[%d]%.*s", t.type, (t.end - t.begin + 1), buf.begin() + t.begin);
  }
  return 0;
} catch (...) {
  return 1;
}
