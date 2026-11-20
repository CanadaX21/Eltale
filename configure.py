import ninja_syntax
import os
import shlex
import sys

dir_path = 'src/'
asm_path = 'asm/'
assets_path = 'assets/'

optO2_files = ['pimgr.c']  # libultra pimgr.o: -O2 without -g

optO1_files = ['initialize.c', 'createthread.c', 'startthread.c', 'createmesgqueue.c', 'visetevent.c']  # libultra os/ objects built -O1 (Quest's build has createthread, startthread, pimgr, createmesgqueue and visetevent -O1; pimgr is -O2 here, above)

optg0_files = ['title_screen.c', 'title_menu.c', 'title_fade.c', 'brian_model.c']  # the title overlay's files (Quest 64's build compiles its title overlay -O2 -g0) and brian_model.c, Brian's model tables

c_files = []
for root, dirs, files in os.walk(dir_path):
    for file in files:
        if file.endswith('.c'):
            c_files.append(os.path.join(root, file))

# the typed assets splat writes as C (the boss files' and monster sets' display lists, vertices, animation records
# and data records) are C units of their own. They hold data only and are compiled without debug information
# (-g0): with -g3 every object repeated every record the asset headers declare (270 KB an object).
ASSET_C = ('.gfx.inc.c', '.vtx.inc.c', '.anim.inc.c', '.records.inc.c', '.maprecords.inc.c', '.spellrecords.inc.c', '.npcrecords.inc.c',
           '.gfxrecords.inc.c')
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith(ASSET_C):
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
            
i8_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.i8.png'):        # not .ci8.png
            i8_files.append(os.path.join(root, file))

# CI8 textures (the spell bank's, splat's ci8tex type): the build reads their colour indices; the palette is a record
ci8_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.ci8.png'):
            ci8_files.append(os.path.join(root, file))

# CI4 textures (the font sheet, splat's ci4tex type): the build reads their colour indices; the palettes are an
# image of their own (assets/font/text_palettes.tlut.rgba16.png)
ci4_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.ci4.png'):
            ci4_files.append(os.path.join(root, file))

# the dialogue bank (splat's dialogue type): each part's source text is encoded at build time (tools/text_build.py)
# into its object and a header of its strings' names (build/include/dialogue/<part>.h), which the records that
# name a string include through include/dialogue.h: every C compile waits for the headers (order-only), and
# a header is rewritten only when a name's value changes
dialogue_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.dialogue.txt'):
            dialogue_files.append(os.path.join(root, file))
dialogue_headers = ['build/include/dialogue/' + os.path.basename(f)[:-len('.dialogue.txt')] + '.h'
                    for f in dialogue_files]

i4_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('i4.png') and not file.endswith('.ci4.png'):    # not .ci4.png
            i4_files.append(os.path.join(root, file))

# the NPC files' name pools (splat's npcnames type): each pool's source text is encoded at build time
# (tools/text_build.py) into an object that defines each name at its string
npcnames_files = []
for root, dirs, files in os.walk(assets_path):
    for file in files:
        if file.endswith('.npcnames.txt'):
            npcnames_files.append(os.path.join(root, file))

j_files = []
j_files.extend([f.replace('.png', '.j') for f in rgba16_files + i8_files + i4_files + ci8_files + ci4_files + ia4_files])
            
# Combine the lists and change file extensions
o_files = []
for file in c_files + s_files + bin_files + ia4_files + rgba16_files + i8_files + i4_files + ci8_files + ci4_files + dialogue_files + npcnames_files:
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
    "include_cflags = -I . -I include -I include/2.0I -I include/2.0I/PR -I build/include\n"
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

# every C declaration of a variable an assembly .bss row reserves must fit the row's space (tools/check_storage.py)
ninja_file.rule('check_storage',
                 command = '$PYTHON tools/check_storage.py > $out.log && mv $out.log $out',
                 description = 'Checking C declarations against the assembly-reserved storage')

# the save format's layout and its machine-readable definition (tools/save_format.py --check against
# tools/save_format.json): a part that outgrows its place in the note, a copy that outgrows its game variable, a
# field saved and restored through different variables, or a definition behind the source stops the build
ninja_file.rule('check_save_format',
                 command = '$PYTHON tools/save_format.py --check > $out.log || { cat $out.log; exit 1; }; mv $out.log $out',
                 description = 'Checking the save format against its definition')

ninja_file.rule('make_elf',
    command = '$LD $LDFLAGS -o $out',
    description = 'Linking ELF')

# Brian's model's link address: splat writes a segment's vram, the address its records are extracted
# and named at, as its section's address in the linker script. Brian's model is placed at brian_model_LINK instead
# (undefined_syms.jp.txt: the original's 0x80206000), so a modified build can lower it to give the model more room:
# every reference into the model is a name, and the link's asserts keep it above the animation buffer's largest file
# and below the overlay slot. The extraction stays at the original address, where the ROM's own pointers name records.
import re as _re
with open('eltail.ld', encoding='utf-8') as _f:
    _ld = _f.read()
_ld, _n = _re.subn(r'(\n\s*\.brian_model )0x80206000( :)', r'\1brian_model_LINK\2', _ld)
if _n != 1 and not _re.search(r'\n\s*\.brian_model brian_model_LINK :', _ld):   # not rewritten by an earlier run
    raise SystemExit('configure.py: the linker script does not place brian_model at 0x80206000 (its yaml vram)')
if _n:
    with open('eltail.ld', 'w', encoding='utf-8') as _f:
        _f.write(_ld)

# the original ROM's SHA-1; when the ROM differs, tools/bss_order.py check says whether a C unit's .bss left the
# original layout (a header or declaration change: `python3 tools/bss_order.py fix` rewrites the BSS_ORDER_PAD
# values; docs/DECLARATIONS.md). A modified build differs on purpose and fails this step as before.
ninja_file.rule('make_z64',
    command = '($OBJCOPY -O binary $in $out) && (sha1sum -c eltail.jp.sha1 || '
              '{ $PYTHON tools/bss_order.py check | grep -E "^(FAIL|bss_order)"; exit 1; })',
    description = 'Making z64')

ninja_file.rule('make_expected',
    command = '(cp $in $out) && (python3 ./$MAKE_EXPECTED $in)')
    
ninja_file.rule('ia4_build',
                 command = "$PYTHON ./$IMG_CONVERT ia4 $in $out",
                 description = "Converting ia4")
                 
ninja_file.rule('rgba16_convert',
                 command = "($PYTHON ./$IMG_CONVERT rgba16 $in $out)",
                 description = "Converting rgba16") 
                 
ninja_file.rule('i8_convert',
                 command = "($PYTHON ./$IMG_CONVERT i8 $in $out)",
                 description = "Converting i8")

ninja_file.rule('i4_convert',
                 command = "($PYTHON ./$IMG_CONVERT i4 $in $out)",
                 description = "Converting i4")

ninja_file.rule('ci8_convert',
                 command = "($PYTHON ./$IMG_CONVERT ci8 $in $out)",
                 description = "Converting ci8")

ninja_file.rule('ci4_convert',
                 command = "($PYTHON ./$IMG_CONVERT ci4 $in $out)",
                 description = "Converting ci4")

ninja_file.rule('dialogue_build',
                 command = '$PYTHON tools/text_build.py $in $out.s $hdr && $AS $ASFLAGS -o $out $out.s',
                 restat = True,
                 description = 'Encoding dialogue text')

ninja_file.rule('npcnames_build',
                 command = '$PYTHON tools/text_build.py $in $out.s && $AS $ASFLAGS -o $out $out.s',
                 description = 'Encoding NPC names')

ninja_file.rule('rgba16_build',
                 command = "($LD -r -b binary -o $out $in)",
                 description = "Converting rgba16")   
print (c_files)

for c_file in c_files:
    if os.path.basename(c_file) in optO2_files:
        ninja_file.build("build/" + append_extension(c_file), "O2_cc", c_file, order_only=dialogue_headers)
    elif os.path.basename(c_file) in optO1_files:
        ninja_file.build("build/" + append_extension(c_file), "O1_cc", c_file, order_only=dialogue_headers)
    elif os.path.basename(c_file) in optg0_files or c_file.endswith(ASSET_C):
        ninja_file.build("build/" + append_extension(c_file), "O2g0_cc", c_file, order_only=dialogue_headers)
    else:
        ninja_file.build("build/" + append_extension(c_file), "main_cc", c_file, order_only=dialogue_headers)
for s_file in s_files:
    if "asm/nonmatchings" in s_file:
        continue
    ninja_file.build("build/" + append_extension(s_file), "s_file", s_file)
for f, h in zip(dialogue_files, dialogue_headers):
    ninja_file.build("build/" + append_extension(f), "dialogue_build", f,
                     implicit=['text/encoding.json', 'tools/text_codec.py', 'tools/text_build.py',
                               'include/quest_types.h'],      # MESSAGE_TEXT_SIZE: text_build.py checks it
                     implicit_outputs=[h], variables={'hdr': h})
for f in npcnames_files:
    ninja_file.build("build/" + append_extension(f), "npcnames_build", f,
                     implicit=['text/encoding.json', 'tools/text_codec.py', 'tools/text_build.py'])
for bin_file in bin_files:
    ninja_file.build("build/" + append_extension(bin_file), "bin_file", bin_file)
    
for rgba16_file in rgba16_files:
    ninja_file.build("build/" + append_extension(rgba16_file, '.j'), "rgba16_convert", rgba16_file)
    
for i8_file in i8_files:
    ninja_file.build("build/" + append_extension(i8_file, '.j'), "i8_convert", i8_file)

for i4_file in i4_files:
    ninja_file.build("build/" + append_extension(i4_file, '.j'), "i4_convert", i4_file)

# IA4 textures (Brian's model): converted and linked as the other images are
for ia4_file in ia4_files:
    ninja_file.build("build/" + append_extension(ia4_file, '.j'), "ia4_build", ia4_file)

for ci8_file in ci8_files:
    ninja_file.build("build/" + append_extension(ci8_file, '.j'), "ci8_convert", ci8_file)

for ci4_file in ci4_files:
    ninja_file.build("build/" + append_extension(ci4_file, '.j'), "ci4_convert", ci4_file)

 #j files are png images converted using image_converter.py
for j_file in j_files:
    ninja_file.build("build/" + os.path.splitext(j_file)[0] + ".png.o", "rgba16_build", "build/" + os.path.splitext(j_file)[0] + ".png.j")
       
# the linker script and the symbol lists are inputs of the link too: an edit to one relinks
storage_inputs = ["tools/check_storage.py", "tools/c_layout.py", "eltail.jp.yaml"] + c_files
for root, dirs, files in os.walk("include"):
    storage_inputs += [os.path.join(root, f) for f in sorted(files) if f.endswith(".h")]
for f in sorted(os.listdir("asm/data")):
    if f.endswith(".bss.s"):
        storage_inputs.append(os.path.join("asm/data", f))
ninja_file.build("build/check_storage.ok", "check_storage", [], implicit=storage_inputs)
save_format_inputs = ["tools/save_format.py", "tools/c_layout.py", "tools/save_format.json",
                      "include/game/save_format.h", "include/quest_types.h", "undefined_syms.jp.txt",
                      "assets/622F0.bin"] + c_files
ninja_file.build("build/check_save_format.ok", "check_save_format", [], implicit=save_format_inputs)
ninja_file.build("build/eltail.jp.elf", "make_elf ", o_files,
                 implicit=["eltail.ld", "undefined_syms_auto.txt", "undefined_funcs_auto.txt", "undefined_syms.jp.txt",
                           "build/check_storage.ok", "build/check_save_format.ok"])
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
