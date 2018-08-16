#!/usr/bin/env python2
import os

if __name__ == "__main__":
    code = []

    for op, name in [
        ("+", "Add"),
        ("-", "Sub"),
        ("*", "Mul"),
        ("/_s", "SDiv"),
        ("/_u", "UDiv"),
        ("%_s", "SRem"),
        ("%_u", "URem")
      ]:

          code.append(
"""void {0}Expr::print(ostream& os, UINT depth) const {{
\tBinaryExpr::print(os, depth, "{1}");
}}\n\n""".format(name, op))

    cur_dir = os.path.abspath(os.path.dirname(__file__))
    with open(os.path.join(cur_dir, "expr.cpp")) as f:
        data = f.read()

    with open(os.path.join(cur_dir, "../expr__gen.cpp"), "w") as f:
        f.write(data.replace("{CODEGEN}", ''.join(code)))
