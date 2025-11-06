export module c42:phase41;
import :tokens;

using namespace c42;

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

static auto take_ident(token_stream & str, token_type type) {
  consume_space(str);

  if (str.peek().type != t_identifier) return take_until_eol(str, t_error);

  auto t = str.take();

  consume_space(str);
  if (str.peek().type != t_new_line) {
    t.type = t_error;

    auto rt = take_until_eol(str, t_error);
    t.end = rt.end;
    return t;
  }

  t.type = type;
  return t;
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

static void take_no_param(token_list & res, token_stream & str, token t, token_type type) {
  t.type = type;
  res.push_back(t);

  if (str.peek().type == t_new_line) {
    t = str.take();
    return;
  }

  res.push_back(take_until_eol(str, t_error));
}

/// Translates preprocessor directives (#, import, export) into custom tokens
auto phase_4_1(const token_list & ctx) {
  auto res = ctx.shallow();
  token_stream str { ctx };
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

        // TODO: mark before and after with extra tokens
        consume_space(str);
      } else if (ctx.txt(t) == "else") {
        take_no_param(res, str, t, t_else);
        continue;
      } else if (ctx.txt(t) == "endif") {
        take_no_param(res, str, t, t_endif);
        continue;
      } else if (ctx.txt(t) == "elifdef") {
        res.push_back(take_ident(str, t_elifdef));
        continue;
      } else if (ctx.txt(t) == "elifndef") {
        res.push_back(take_ident(str, t_elifndef));
        continue;
      } else if (ctx.txt(t) == "error") {
        res.push_back(take_until_eol(str, t_error));
        continue;
      } else if (ctx.txt(t) == "ifdef") {
        res.push_back(take_ident(str, t_ifdef));
        continue;
      } else if (ctx.txt(t) == "ifndef") {
        res.push_back(take_ident(str, t_ifndef));
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
      // TODO: transform into [-8]name[-X]part or similar
      res.push_back(take_until_semi(str, t_import));
      continue;
    } else if (t.type == t_module) {
      // TODO: transform into [-9]name[-X]part or similar
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

