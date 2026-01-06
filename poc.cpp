#pragma leco tool
import c42;
import jojo;
import jute;
import print;
import sv;

struct defs : c42::defines {
  bool has(sv name) const override {
    return name == "_WIN32";
  }
};

int main() try {
  defs d {};

  jute::view fn = "tests/preproc.cppm";
  auto buf = jojo::slurp(fn);
  auto ctx = c42::preprocess(&d, buf);

  const auto log = [&](auto t, jute::view lvl) {
    errln(fn, ":", t.line, ":", t.column, ": [", lvl, "] ", ctx.txt(t));
  };

  bool has_error = false;
  for (auto t : ctx) {
    if (t.type == c42::t_warning) {
      log(t, "warning");
    } else if (t.type == c42::t_error) {
      log(t, "error");
      has_error = true;
    }
  }
  if (has_error) return 1;

  for (auto t : ctx) {
    putf("[%d]", t.type);
    put(ctx.txt(t));
  }
  return 0;
} catch (...) {
  return 1;
}
