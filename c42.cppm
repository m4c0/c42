export module c42;
import :phases13;
import :tokens;
import hai;
import sv;
import traits;

using namespace c42;

class context {
  const char * m_orig_src;
  hai::chain<token> m_t; 

public:
  constexpr context(const char * orig_src, hai::chain<token> t)
    : m_orig_src { orig_src }
    , m_t { traits::move(t) }
  {}

  [[nodiscard]] context shallow() const { return context { m_orig_src, { m_t.size() } }; }
  [[nodiscard]] token_stream stream() const { return token_stream { m_t }; }
  [[nodiscard]] sv txt(token t) const {
    return { m_orig_src + t.begin, t.end - t.begin + 1 };
  }

  [[nodiscard]] auto begin() const { return m_t.begin(); }
  [[nodiscard]] auto end() const { return m_t.end(); }

  void push_back(token t) { m_t.push_back(t); }
};

static void consume_space(token_stream &str) {
  while (str.peek().type == t_space) {
    str.skip(1);
  }
}

static auto take_until_eol(token_stream & str, token_type type) {
  consume_space(str);

  auto t = str.take();
  if (t.type == t_new_line) {
    t.type = t_error;
    return t;
  }

  auto rt = t;
  auto nt = t;
  while (str.has_more() && t.type != t_new_line) {
    nt = t;
    t = str.take();
  }
  rt.type = type;
  rt.end = nt.end;
  return rt;
}

static auto take_until_semi(token_stream & str, token_type type) {
  consume_space(str);

  auto t = str.take();
  if (t.type == ';') {
    t.type = type;
    t.end--;
    return t;
  }

  auto rt = t;
  auto nt = t;
  while (str.has_more() && t.type != ';') {
    nt = t;
    t = str.take();
  }
  rt.type = type;
  rt.end = nt.end;
  return rt;
}

static void take_no_param(context & res, token_stream & str, token t, token_type type) {
  t.type = type;
  res.push_back(t);

  if (str.peek().type == t_new_line) {
    t = str.take();
    return;
  }

  res.push_back(take_until_eol(str, t_error));
}

/// Translates preprocessor directives (#, import, export) into custom tokens
static auto phase_4_1(const context & ctx) {
  context res = ctx.shallow();
  auto str = ctx.stream();
  while (str.has_more()) {
    consume_space(str);

    auto t = str.take();
    if (t.type == '#') {
      consume_space(str);

      if (!str.has_more()) break;

      t = str.take();
      if (t.type != t_identifier) {
        t.type = t_error;
      } else if (ctx.txt(t) == "define") {
        consume_space(str);

        t = str.take();
        t.type = t.type == t_identifier ? t_define : t_error;

        consume_space(str);
      } else if (ctx.txt(t) == "else") {
        take_no_param(res, str, t, t_else);
        continue;
      } else if (ctx.txt(t) == "endif") {
        take_no_param(res, str, t, t_endif);
        continue;
      } else if (ctx.txt(t) == "error") {
        res.push_back(take_until_eol(str, t_error));
        continue;
      } else if (ctx.txt(t) == "include") {
        // TODO: check if "this" or <that>
        res.push_back(take_until_eol(str, t_include));
        continue;
      } else if (ctx.txt(t) == "pragma") {
        res.push_back(take_until_eol(str, t_pragma));
        continue;
      } else if (ctx.txt(t) == "warning") {
        res.push_back(take_until_eol(str, t_warning));
        continue;
      } else {
        t.type = t_directive;
        consume_space(str);
      }
    } else if (t.type == t_identifier && ctx.txt(t) == "export") {
      t.type = t_export;
      res.push_back(t);
      continue; // process next token as if it wasn't exported
    } else if (t.type == t_import) {
      res.push_back(take_until_semi(str, t_import));
      continue;
    } else if (t.type == t_module) {
      res.push_back(take_until_semi(str, t_module));
      continue;
    }

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

/// Process supported directives
static auto phase_4_2(const context & ctx) {
  context res = ctx.shallow();
  auto str = ctx.stream();
  while (str.has_more()) {
    auto t = str.take();

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

static auto phase_4(const context & ctx) {
  return phase_4_2(phase_4_1(ctx));
}

export namespace c42 {
  auto preprocess(sv buf) {
    context ctx { buf.begin(), phase_3(phase_2(phase_1(buf))) };
    return phase_4(ctx);
  }
}
