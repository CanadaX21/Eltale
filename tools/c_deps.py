"""c_deps.py OUT SOURCE [CPP_ARGS...]: write OUT.d, the dependency list of the C file SOURCE for ninja
(configure.py's C rules: `depfile = $out.d`, `deps = gcc`), so an incremental build recompiles a C file whenever a
file it reads changes:
  - the headers it includes, as the host C preprocessor finds them with the build's include paths and defines
    (CPP_ARGS; the preprocessor only lists them: IDO compiles);
  - the assembly files its `#pragma GLOBAL_ASM("...")` lines name and the files its `.incbin` lines read, which
    asm-processor includes at compile time.
The preprocessor is $CPP if set, else cpp. Standard library only."""
import os, re, subprocess, sys


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    out, src, cpp_args = sys.argv[1], sys.argv[2], sys.argv[3:]
    cpp = os.environ.get('CPP') or 'cpp'
    res = subprocess.run([cpp, '-MM', '-MT', out] + cpp_args + [src], capture_output=True, text=True)
    if res.returncode != 0:
        sys.stderr.write(res.stderr)
        sys.exit(res.returncode)
    text = res.stdout.replace('\\\n', ' ')
    deps = text.split(':', 1)[1].split() if ':' in text else [src]
    with open(src, encoding='utf-8', errors='replace') as f:
        body = f.read()
    for path in (re.findall(r'^[ \t]*#[ \t]*pragma[ \t]+GLOBAL_ASM\(\s*"([^"]+)"\s*\)', body, re.M) +
                 re.findall(r'^[ \t]*\.incbin[ \t]+"([^"]+)"', body, re.M)):
        if path not in deps:
            deps.append(path)
    with open(out + '.d', 'w', encoding='utf-8') as f:
        f.write('%s: %s\n' % (out, ' '.join(deps)))


if __name__ == '__main__':
    main()
