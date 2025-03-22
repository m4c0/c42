export module c42;
import :phases13;
import :tokens;
import hai;
import jute;

using namespace c42;
using namespace jute::literals;

static void consume_space(token_stream &str) {
  while (str.peek().type == t_space) {
    str.skip(1);
  }
}

/// Translates preprocessor directives (#, import, export) into custom tokens
static auto phase_4_1(const hai::chain<token> & t) {
  hai::chain<token> res { t.size() };
  token_stream str{t};
  while (str.has_more()) {
    consume_space(str);

    auto t = str.take();
    if (t.type == '#') {
      consume_space(str);

      if (!str.has_more()) break;

      t = str.take();
      t.type = t_directive;
      consume_space(str);
    } else if (t.type == t_import) {
      consume_space(str);
    } else if (t.type == t_module) {
      consume_space(str);
    }

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

static auto phase_4_2(const char * buf, const hai::chain<token> & t) {
  hai::chain<token> res { t.size() };
  token_stream str { t };
  while (str.has_more()) {
    auto t = str.take();

    if (t.type == t_directive) {
      auto txt = jute::view { buf + t.begin, t.end - t.begin + 1 };
      if (txt == "error" || txt == "warning") {
        auto t = str.take();
        auto rt = t;
        auto nt = t;
        while (str.has_more() && t.type != t_new_line) {
          nt = t;
          t = str.take();
        }
        rt.type = txt == "error" ? t_error : t_warning;
        rt.end = nt.end;
        rt.value = jute::view { buf + rt.begin, rt.end - rt.begin + 1 };
        res.push_back(rt);
        continue;
      } else if (txt == "embed") {
        while (str.has_more()) {
          consume_space(str);
          t = str.take();
          if (t.type == t_new_line) break;
          if (t.type != t_str) {
            auto nt = t;
            nt.type = t_error;
            nt.value = "Embeddable filenames must be strings"_hs;
            res.push_back(nt);
            continue;
          }
          t.type = t_embed;
          res.push_back(t);
        }
        continue;
      }
    }

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

static auto phase_4(const char * buf, const hai::chain<token> & t) {
  return phase_4_2(buf, phase_4_1(t));
}

export namespace c42 {
  auto preprocess(const hai::cstr & buf) {
    return phase_4(buf.begin(), phase_3(phase_2(phase_1(buf))));
  }
}
