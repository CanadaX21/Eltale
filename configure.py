import ninja_syntax
import os
import shlex
import sys

dir_path = 'src/'
asm_path = 'asm/'
assets_path = 'assets/'

optO2_files = ['pimgr.c']  # libultra pimgr.o: -O2 without -g

optO1_files = ['initialize.c', 'createthread.c', 'startthread.c', 'createmesgqueue.c', 'visetevent.c']  # libultra os/ objects built -O1 (as Quest: createthread, startthread, pimgr, createmesgqueue, visetevent)

optg0_files = ['title_screen.c', 'title_menu.c', 'title_fade.c']  # the title overlay: Quest 64's build compiles its title overlay -O2 -g0

c_files = []
for root, dirs, files in os.walk(dir_path):
    for file in files:
        if file.endswith('.c'):
            c_files.append(os.path.join(root, file))

# the typed assets splat writes as C (the boss files' display lists and vertices) are C units of their own
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.gfx.inc.c') or file.endswith('.vtx.inc.c'):
            c_files.append(os.path.join(root, file))

s_files = []
for root, dirs, files in os.walk(asm_path):
    for file in files:
        if file.endswith('.s'):
            s_files.append(os.path.join(root, file))

bin_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.bin'):
            bin_files.append(os.path.join(root, file))

def append_extension(filename, extension='.o'):
    return filename + extension

ia4_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('ia4.png'):
            ia4_files.append(os.path.join(root, file))
            
rgba16_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('rgba16.png'):
            rgba16_files.append(os.path.join(root, file))
            
j_files = []
j_files.extend([f.replace('.png', '.j') for f in rgba16_files])
            
# Combine the lists and change file extensions
o_files = []
for file in c_files + s_files + bin_files + ia4_files + rgba16_files:
    if 'asm/nonmatchings/' not in file:
        o_files.append("build/" + append_extension(file))

header = (
    "AS = mips-linux-gnu-as\n"
    "CPP = cpp\n"
    "LD = mips-linux-gnu-ld\n"
    "OBJDUMP = mips-linux-gnu-objdump\n"
    "OBJCOPY = mips-linux-gnu-objcopy\n"
    "OBJCOPYFLAGS = -O binary\n"
    # the Python that runs this script (the .venv's when ./configure runs it) also runs the build's Python tools:
    # the image converter needs pypng, which a fresh setup installs only into the .venv (quoted for the shell, $
    # escaped for ninja: it is the build's one absolute path)
    "PYTHON = " + ninja_syntax.escape(shlex.quote(sys.executable)) + "\n"
    "cflags = -G 0 -Xfullwarn -Xcpluscomm -signed -nostdinc -non_shared -Wab,-r4300_mul -D_LANGUAGE_C -D_FINALROM -DF3DEX_GBI -DWIN32 -woff 649,838\n"
    "include_cflags = -I . -I include -I include/2.0I -I include/2.0I/PR\n"
    "ido_cc = tools/ido_5.3/usr/lib/cc\n"
    "mips_version = -mips2 -o32\n"
    "DEFINES = -D_LANGUAGE_C -DF3DEX_GBI -DNDEBUG\n"
    "CFLAGS = -woff 649,838,513 $include_cflags\n"
    "DEPFLAGS = -nostdinc -D_LANGUAGE_C -D_FINALROM -DF3DEX_GBI -DWIN32 -DNDEBUG $include_cflags\n"
    "ASM_PROC = python3 tools/asm-processor/build.py\n"
    "ASM_PROC_FLAGS = --input-enc=utf-8 --output-enc=euc-jp\n"
    "ASFLAGS = -EB -mtune=vr4300 -march=vr4300 -mabi=32 -Iinclude -Isrc\n"
    "XGCC = mips-linux-gnu-gcc\n"
    "IMG_CONVERT = tools/image_converter.py\n"
    "MAKE_EXPECTED = tools/make_expected.py\n"
    "LD_SCRIPT = eltail.ld\n"
    "LDFLAGS = -T $LD_SCRIPT -Map build/eltail.jp.map -T undefined_syms_auto.txt -T undefined_funcs_auto.txt -T undefined_syms.jp.txt --no-check-sections\n"
)

# Create a Ninja build file object
with open('build.ninja', 'w') as file:
    file.write(f'{header}')
ninja_file = ninja_syntax.Writer(open('build.ninja', 'a'))

# Header dependencies: before each compile, tools/c_deps.py lists the files the C file reads (its headers, as the
# host preprocessor finds them with DEPFLAGS, and the assembly its GLOBAL_ASM lines include); ninja keeps the lists
# (deps = gcc), so an incremental build recompiles every file that includes an edited header
C_DEPS = '$PYTHON tools/c_deps.py $out $in $DEPFLAGS $dep_include && '

#has -g3
ninja_file.rule('main_cc',
    depfile = '$out.d', deps = 'gcc',
    command = C_DEPS + '$ASM_PROC $ASM_PROC_FLAGS $ido_cc -- $AS $ASFLAGS -- -c -32 $cflags $DEFINES $CFLAGS -O2 -g3 $mips_version -o $out $in',
    description = 'Compiling -O2 .c file' )
#doesn't have -g3
ninja_file.rule('O2g0_cc',
    depfile = '$out.d', deps = 'gcc',
    command = C_DEPS + '$ASM_PROC $ASM_PROC_FLAGS $ido_cc -- $AS $ASFLAGS -- -c -32 $cflags $DEFINES $CFLAGS -O2 -g0 $mips_version -o $out $in',
    description = 'Compiling -O2 -g0 .c file')

ninja_file.rule('O2_cc',
    depfile = '$out.d', deps = 'gcc',
    command = C_DEPS + '$ASM_PROC $ASM_PROC_FLAGS $ido_cc -- $AS $ASFLAGS -- -c -32 $cflags $DEFINES $CFLAGS -O2 $mips_version -o $out $in',
    description = 'Compiling -O2 .c file' )

ninja_file.rule('O1_cc',
    depfile = '$out.d', deps = 'gcc',
    command = C_DEPS + '$ASM_PROC $ASM_PROC_FLAGS $ido_cc -- $AS $ASFLAGS -- -c -32 $cflags $DEFINES $CFLAGS -O1 $mips_version -o $out $in',
    description = 'Compiling -O1 .c file' )
    
ninja_file.rule('s_file',
    depfile = '$out.d', deps = 'gcc',
    command = 'iconv --from UTF-8 --to EUC-JP $in | $AS $ASFLAGS --MD $out.d -o $out',
    description = 'Assembling .s file' )

ninja_file.rule('bin_file',
    command = '$LD -r -b binary -o $out $in')

ninja_file.rule('make_elf',
    command = '$LD $LDFLAGS -o $out',
    description = 'Linking ELF')

ninja_file.rule('make_z64',
    command = '($OBJCOPY -O binary $in $out) && (sha1sum -c eltail.jp.sha1)',
    description = 'Making z64')

ninja_file.rule('make_expected',
    command = '(cp $in $out) && (python3 ./$MAKE_EXPECTED $in)')
    
ninja_file.rule('ia4_build',
                 command = "$PYTHON ./$IMG_CONVERT ia4 $in $out",
                 description = "Converting ia4")
                 
ninja_file.rule('rgba16_convert',
                 command = "($PYTHON ./$IMG_CONVERT rgba16 $in $out)",
                 description = "Converting rgba16") 
                 
ninja_file.rule('rgba16_build',
                 command = "($LD -r -b binary -o $out $in)",
                 description = "Converting rgba16")   
print (c_files)

for c_file in c_files:
    if os.path.basename(c_file) in optO2_files:
        ninja_file.build("build/" + append_extension(c_file), "O2_cc", c_file)
    elif os.path.basename(c_file) in optO1_files:
        ninja_file.build("build/" + append_extension(c_file), "O1_cc", c_file)
    elif os.path.basename(c_file) in optg0_files:
        ninja_file.build("build/" + append_extension(c_file), "O2g0_cc", c_file)
    else:
        ninja_file.build("build/" + append_extension(c_file), "main_cc", c_file)
for s_file in s_files:
    if "asm/nonmatchings" in s_file:
        continue
    ninja_file.build("build/" + append_extension(s_file), "s_file", s_file)
for bin_file in bin_files:
    ninja_file.build("build/" + append_extension(bin_file), "bin_file", bin_file)
    
for rgba16_file in rgba16_files:
    ninja_file.build("build/" + append_extension(rgba16_file, '.j'), "rgba16_convert", rgba16_file)
    
 #j files are png images converted using image_converter.py
for j_file in j_files:
    ninja_file.build("build/" + os.path.splitext(j_file)[0] + ".png.o", "rgba16_build", "build/" + os.path.splitext(j_file)[0] + ".png.j")
       
# the linker script and the symbol lists are inputs of the link too: an edit to one relinks
ninja_file.build("build/eltail.jp.elf", "make_elf ", o_files,
                 implicit=["eltail.ld", "undefined_syms_auto.txt", "undefined_funcs_auto.txt", "undefined_syms.jp.txt"])
ninja_file.build("build/eltail.jp.z64", "make_z64 ", "build/eltail.jp.elf")
ninja_file.build("build/eltail.jp.ok", "make_expected ", "build/eltail.jp.z64")
# plain `ninja` builds the ROM, checks it against eltail.jp.sha1 and keeps the matching build in expected/
# (tools/make_expected.py); the objdiff targets added below are built only when asked for
ninja_file.default("build/eltail.jp.ok")


print ("build.ninja generated")
ninja_file.close()

# objdiff support (tools/objdiff_gen.py): objdiff.json and optional per-unit targets
import subprocess, sys
subprocess.run([sys.executable, "tools/objdiff_gen.py"], check=True)
