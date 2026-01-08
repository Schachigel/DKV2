import os
import re

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
problem_files = []
for dirpath, dirs, files in os.walk(root):
    if '/build/' in dirpath.replace('\\','/'):
        continue
    for f in files:
        if not f.endswith('.cpp'):
            continue
        path = os.path.join(dirpath, f)
        basename = os.path.splitext(f)[0]
        own_header = f'{basename}.h'
        with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
            lines = [next(fh) for _ in range(200)] if True else fh.readlines()
        # find all quoted includes
        quoted_includes = [re.match(r'\s*#include\s+"([^"]+)"', l) for l in lines]
        quoted_includes = [m.group(1) for m in quoted_includes if m]
        if not quoted_includes:
            continue
        # check if own_header present
        if own_header in quoted_includes:
            if quoted_includes[0] != own_header:
                problem_files.append((path, own_header, quoted_includes))
        else:
            # own header missing: maybe no header required; skip
            pass

if not problem_files:
    print('OK: no problems found')
else:
    print('Files needing reorder:')
    for p, h, incs in problem_files:
        print(p)
        print('  own header:', h)
        print('  includes order:', incs[:6])
