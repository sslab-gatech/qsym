#!/usr/bin/env python2
import os

def parse_func(func):
  func = func.strip().replace("virtual ", "").replace("ExprRef ", "", 1)[:-1]
  name, args = func.split("(", 1)
  args = args[:-1] # remove ')'
  args_name = [arg.split(" ")[1] for arg in args.split(", ")]
  return func, name, args_name

def read_def(name):
  dirp = os.path.abspath(os.path.dirname(__file__))
  header = os.path.join(dirp, "../expr_builder.h")

  f = open(header, "r")
  funcs = []
  begin = False

  for l in f:
    # remove the override keyword
    l = l.replace(" override", "")
    if "{END:%s}" % name in l:
      break
    if begin:
      funcs.append(parse_func(l))
    if "{BEGIN:%s}" % name in l:
      begin = True
  f.close()
  return funcs

if __name__ == '__main__':
    code = []
    funcs = read_def("FUNC")

    # generate ExprBuilder::create*
    for func, name, args_name in funcs:
        code.append(
"""ExprRef ExprBuilder::{0}
{{
\treturn next_->{1}({2});
}}\n\n""".format(func, name, ', '.join(args_name)))

    BINARY_KIND = [
    "Add", "Sub", "Mul", "UDiv", "SDiv", "URem", "SRem", "And", "Or", "Xor",
    "Shl", "LShr", "AShr", "Equal", "Distinct", "Ult", "Ule", "Ugt", "Uge",
    "Slt", "Sle", "Sgt", "Sge", "LOr", "LAnd"]

    # generate createBinary
    code.append(
"""ExprRef ExprBuilder::createBinaryExpr(Kind kind, ExprRef l, ExprRef r) {\n
\tswitch (kind) {\n""");

    for kind in BINARY_KIND:
        code.append(
"""\t\tcase {0}:
\t\t\treturn create{0}(l, r);\n""".format(kind))

    code.append(
"""\t\tdefault:
\t\t\tLOG_FATAL("Non-binary expr: " + std::to_string(kind) + "\\n");
\t\t\treturn NULL;
\t}
}\n\n""");

    # generate createUnary
    UNARY_KIND = ["Not", "Neg", "LNot"]
    code.append(
"""ExprRef ExprBuilder::createUnaryExpr(Kind kind, ExprRef e) {
\tswitch (kind) {""")
    for kind in UNARY_KIND:
        code.append(
"""\t\tcase {0}:
\t\t\treturn create{0}(e);\n""".format(kind))

    code.append(
"""\t\tdefault:
\t\t\tLOG_FATAL("Non-unary expr: " + std::to_string(kind) + "\\n");
\t\t\treturn NULL;
\t}
}\n\n""");

    # generate BaseExprBuilder
    for func, name, args_name in read_def("BASE"):
      expr_name = name[len("create"):] + "Expr"
      code.append(
"""ExprRef BaseExprBuilder::{0} {{
\tExprRef ref = std::make_shared<{1}>({2});
\taddUses(ref);
\treturn ref;
}}\n\n""".format(func, expr_name, ', '.join(args_name)))

    # generate CacheExprBuilder
    for func, name, args_name in read_def("CACHE"):
      code.append(
"""ExprRef CacheExprBuilder::{0} {{
\tExprRef new_expr = ExprBuilder::{1}({2});
\treturn findOrInsert(new_expr);
}}\n\n""".format(func, name, ', '.join(args_name)))

    # generate CommutativeExprBuilder
    commutative_mapping = {
      "Ult": "Ugt",
      "Ule": "Uge",
      "Ugt": "Ult",
      "Uge": "Ule",
      "Slt": "Sgt",
      "Sle": "Sge",
      "Sgt": "Slt",
      "Sge": "Sle"
    }

    for _, name, _ in read_def("COMMUTATIVE"):
      assert(name.startswith("create"))

      op = name[len("create"):]
      if op in commutative_mapping:
        modified_name = name.replace(op, commutative_mapping[op])
      else:
        modified_name = name

      code.append(
"""ExprRef CommutativeExprBuilder::{0}(ExprRef l, ExprRef r) {{
\tNonConstantExprRef nce_l = castAs<NonConstantExpr>(l);
\tConstantExprRef ce_r = castAs<ConstantExpr>(r);

\tif (nce_l != NULL && ce_r != NULL)
\t\treturn {1}(ce_r, nce_l);

\treturn ExprBuilder::{0}(l, r);
}}\n\n""".format(name, modified_name))

    # generate ConstantFoldingExprBuilder
    OP_BINARY = 1
    OP_FUNC = 2

    RET_CONST = 3
    RET_BOOL = 4

    BINARY_ARITH = {
      # const
      "Add": (RET_CONST, OP_BINARY, "+"),
      "Sub": (RET_CONST, OP_BINARY, "-"),
      "Mul": (RET_CONST, OP_BINARY, "*"),
      "UDiv": (RET_CONST, OP_FUNC,),
      "SDiv": (RET_CONST, OP_FUNC,),
      "URem": (RET_CONST, OP_FUNC,),
      "SRem": (RET_CONST, OP_FUNC,),
      "And": (RET_CONST, OP_BINARY, "&"),
      "Or": (RET_CONST, OP_BINARY, "|"),
      "Xor": (RET_CONST, OP_BINARY, "^"),
      "Shl": (RET_CONST, OP_BINARY, "<<"),
      "LShr": (RET_CONST, OP_FUNC,),
      "AShr": (RET_CONST, OP_FUNC,),

      # bool
      "Ult": (RET_BOOL, OP_FUNC,),
      "Ule": (RET_BOOL, OP_FUNC,),
      "Ugt": (RET_BOOL, OP_FUNC,),
      "Uge": (RET_BOOL, OP_FUNC,),
      "Slt": (RET_BOOL, OP_FUNC,),
      "Sle": (RET_BOOL, OP_FUNC,),
      "Sgt": (RET_BOOL, OP_FUNC,),
      "Sge": (RET_BOOL, OP_FUNC,),

        #"LAnd": (RET_BOOL, OP_BINARY, "&&"),
        #"LOr": (RET_BOOL, OP_BINARY, "||"),
    }

    for k in sorted(BINARY_ARITH.keys()):
      v = BINARY_ARITH[k]
      ret, op = v[:2]
      if op == OP_BINARY:
        stmt = "ce_l->value() {0} ce_r->value()".format(v[2])
      else:
        stmt = "ce_l->value().{0}(ce_r->value())".format(k.lower())

      if ret == RET_CONST:
        stmt = "createConstant({0}, l->bits())".format(stmt)
      else:
        stmt = "createBool({0})".format(stmt)

      code.append(
"""ExprRef ConstantFoldingExprBuilder::create{0}(ExprRef l, ExprRef r) {{
\tConstantExprRef ce_l = castAs<ConstantExpr>(l);
\tConstantExprRef ce_r = castAs<ConstantExpr>(r);

\tif (ce_l != NULL && ce_r != NULL) {{
\t\tQSYM_ASSERT(l->bits() == r->bits());
\t\treturn {1};
\t}}
\telse
\treturn ExprBuilder::create{0}(l, r);
}}\n\n""".format(k,  stmt))


    # generate PruneExprBuilder
    for func, name, args_name in read_def("FUZZ"):
      expr_name = name[len("create"):] + "Expr"
      code.append(
"""ExprRef PruneExprBuilder::{0} {{
\tExprRef ref = ExprBuilder::{1}({2});
\tg_call_stack_manager.updateBitmap();
\tif (g_call_stack_manager.isInteresting())
\t\treturn ref;
\telse
\t\treturn ref->evaluate();
}}\n\n""".format(func, name, ', '.join(args_name)))

    dirp = os.path.abspath(os.path.dirname(__file__))
    header = os.path.join(dirp, "../expr_builder.h")

    cur_dir = os.path.abspath(os.path.dirname(__file__))
    with open(os.path.join(cur_dir, "expr_builder.cpp")) as f:
        data = f.read()

    with open(os.path.join(cur_dir, "../expr_builder__gen.cpp"), "w") as f:
        f.write(data.replace("{CODEGEN}", ''.join(code)))
