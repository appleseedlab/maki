from subprocess import CompletedProcess, run


class Program:

    def __init__(self,
                 name: str,
                 archive_url: str,
                 src_dir: str,
                 configure_script: str,
                 build_script: str,
                 ):
        self.name = name
        self.archive_url = archive_url
        self.src_dir = src_dir
        self.configure_script = configure_script
        self.build_script = build_script

        self.archive_file = archive_url.rsplit(r'/', 1)[1]

    def configure(self) -> CompletedProcess:
        return run(self.configure_script, shell=True)

    def build(self) -> CompletedProcess:
        return run(self.build_script, shell=True)


PROGRAMS = [

    Program(
        r'bash-5.2-rc1',
        r'https://mirror.us-midwest-1.nexcess.net/gnu/bash/bash-5.2-rc1.tar.gz',
        r'.',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # requires makeinfo
    Program(
        r'bc-1.07.1',
        r'https://gnu.mirror.constant.com/bc/bc-1.07.1.tar.gz',
        r'bc',
        r'./configure',
        r'intercept-build make -j8'
    ),

    Program(
        r'bison-3.8.2',
        r'https://mirrors.nav.ro/gnu/bison/bison-3.8.2.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # cvs redeclares stdio's getline function in lib/getline.h,
    # and redefines it in lib/getline.c.
    # to get cvs to compile, i had to rename the function's declaration
    # and definition to something else.
    Program(
        r'cvs-1.11.21',
        r'https://cfhcable.dl.sourceforge.net/project/ccvs/CVS%20Stable%20Source%20Release/1.11.21/cvs-1.11.21.tar.gz',
        r'src',
        r'''
        sed -i 's/getline __PROTO/getline_cvs __PROTO/' lib/getline.h                               &&
        sed -i 's/getline (lineptr, n, stream)/getline_cvs (lineptr, n, stream)/' lib/getline.c     &&
        bash configure
        ''',
        r'intercept-build make'
    ),

    # requires gnutls libjpeg libgif/libungif libtiff gnutls
    # but these requirements can be circumvented with configuration options.
    Program(
        r'emacs-28.1',
        r'https://ftp.snt.utwente.nl/pub/software/gnu/emacs/emacs-28.1.tar.gz',
        r'src',
        r'./configure --with-gnutls=ifavailable --with-gif=ifavailable --with-tiff=ifavailable',
        r'intercept-build make -j8'
    ),

    # # note: this is genscript from the ernst study
    # # the g stands for GNU
    Program(
        r'enscript-1.6.6',
        r'https://ftp.gnu.org/gnu/enscript/enscript-1.6.6.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    Program(
        r'flex-2.6.4',
        r'https://github.com/westes/flex/files/981163/flex-2.6.4.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # requires X11 libraries (i think libx11-dev)
    Program(
        r'fvwm-2.6.9',
        r'https://github.com/fvwmorg/fvwm/releases/download/2.6.9/fvwm-2.6.9.tar.gz',
        r'fvwm',
        r'./configure',
        r'intercept-build make -j8'
    ),

    Program(
        r'gawk-5.1.1',
        r'https://ftp.gnu.org/gnu/gawk/gawk-5.1.1.tar.gz',
        r'.',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # contains c++ code.
    # Program(
    #     r'gcc-12.1.0',
    #     r'https://bigsearcher.com/mirrors/gcc/releases/gcc-12.1.0/gcc-12.1.0.tar.gz',
    #     r'.',
    #     r'./configure --disable-multilib',
    #     r'intercept-build make -j8'
    # ),

    # contains c++ code.
    # Program(
    #     r'ghostscript-9.56.1',
    #     r'https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs9561/ghostscript-9.56.1.tar.gz',
    #     r'',
    #     r'bash configure',
    #     r'intercept-build make -j8'
    # ),

    # requires help2man
    # GNU chess is written in a mix of c and c++ code.
    # Program(
    #     r'gnuchess-6.2.9',
    #     r'https://gnu.mirror.constant.com/chess/gnuchess-6.2.9.tar.gz',
    #     r'src',
    #     r'./configure',
    #     r'intercept-build make -j8'
    # ),

    Program(
        r'gnuplot-5.4.4',
        r'https://cytranet.dl.sourceforge.net/project/gnuplot/gnuplot/5.4.4/gnuplot-5.4.4.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # requires xaw3dg-dev
    # intercept-build didn't initially work because some of the files in gv
    # had utf-8 characters, but were not utf-8 encoded.
    # i fixed this by changing the file encodings to utf-8.
    Program(
        r'gv-3.7.4',
        r'https://mirrors.sarata.com/gnu/gv/gv-3.7.4.tar.gz',
        r'src',
        r'''
        iconv -f ISO-8859-1 -t UTF-8 src/Makefile.am -o tmp && mv -f tmp src/Makefile.am    &&
        iconv -f ISO-8859-1 -t UTF-8 src/Makefile.in -o tmp && mv -f tmp src/Makefile.in    &&
        for FN in src/gv_copyright.dat src/gv_font_res.dat src/gv_font_res-I18N_mb.dat src/gv_layout_res.dat src/gv_misc_res.dat src/gv_spartan.dat src/gv_user_res.dat src/gv_widgetless.dat; do iconv -f US-ASCII -t UTF-8 $FN -o tmp && mv -f tmp $FN; done  &&
        ./configure
        ''',
        r'intercept-build make -j8'
    ),

    Program(
        r'gzip-1.12',
        r'https://mirrors.tripadvisor.com/gnu/gzip/gzip-1.12.tar.gz',
        r'.',
        r'./configure',
        r'intercept-build make -j8',
    ),

    Program(
        r'linux-6.2-rc2',
        r'https://github.com/torvalds/linux/archive/refs/tags/v6.2-rc2.tar.gz',
        r'.',
        r'make CC=clang-14 defconfig',
        r'intercept-build make -j8'
    ),

    Program(
        r'lua-5.4.4',
        r'https://www.lua.org/ftp/lua-5.4.4.tar.gz',
        r'src',
        r'',
        r'intercept-build make -j8',
    ),

    # requires help2man
    Program(
        r'm4-1.4.19',
        r'https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # requires  build-essential libmotif-dev libjpeg62-dev
    #           libxmu-headers libxpm-dev libxmu-dev libpng-dev
    # won't compile with gcc-11.
    # need to use gcc-9 or gcc-10.
    Program(
        r'ncsa-mosaic-af1c9aaaa299da3540faa16dcab82eb681cf624e',
        r'https://github.com/alandipert/ncsa-mosaic/archive/af1c9aaaa299da3540faa16dcab82eb681cf624e.zip',
        r'src',
        r'',
        r'intercept-build make CC=gcc-9 linux -j8',
    ),

    Program(
        r'perl-5.36.0',
        r'https://www.cpan.org/src/5.0/perl-5.36.0.tar.gz',
        r'.',
        r'./Configure -d -e -s',
        r'intercept-build make -j8'
    ),

    # found it!
    # do you know how hard it is to find something on the Internet
    # with such a generic name as "plan" ? :-)

    # FIXME: FTP download for plan does not always work
    # requires libmotif-dev
    # i had a hard time installing this one the remote machine.
    # my solution was to download it using wget --no-passive on my local
    # machine, then scp it over to the remote machine.
    # for some reason, when after configuring and making plan, i run into
    # a redefinition error. yylineno is redefined in lex.yy.c.
    # this occurs in the original, untransformed program.
    # i fixed this by changing the definition of int yylineno in holiday.c
    # to an extern int.
    # Program(
    #     r'plan-1.12',
    #     r'ftp://ftp.bitrot.de/plan/plan-1.12.tar.gz',
    #     r'src',
    #     r'''
    #     cd src                                                      &&
    #     sed -i 's/\<int yylineno\>/extern int yylineno/' holiday.c  &&
    #     ./configure 4                                               &&
    #     make clean
    #     ''',
    #     r'''
    #     cd src                          &&
    #     intercept-build make linux64    &&
    #     mv compile_commands.json ..     &&
    #     cd ..
    #     '''
    # ),

    # FIXME: CANNOT COMPILE
    # # requires libcqrlib-dev libcneartree-dev libcvector-dev libforms-dev
    # Program(
    #     r'RasMol-2.7.5.2',
    #     r'http://www.rasmol.org/software/RasMol_Latest.tar.gz',
    #     r'src',
    #     r'''
    #     cd src                                      &&
    #     ./rasmol_build_options.sh --cbflib_local    &&
    #     xmkmf
    #     ''',
    #     r'''
    #     intercept-build make -j8        &&
    #     mv compile_commands.json ..     &&
    #     cd ..
    #     ''',
    # ),

    # when i try to compile rcs, i get a warning that gets is used
    # where fgets should be used instead.
    # i fixed this problem changing lib/stdio.in.h
    # https://www.fatalerrors.org/a/gets-undeclared-here-not-in-a-function.html
    Program(
        r'rcs-5.10.0',
        r'https://mirror2.sandyriver.net/pub/software/gnu/rcs/rcs-5.10.0.tar.xz',
        r'src',
        r'''
        sed -i 's/SIGSTKSZ)/14528)/' src/b-isr.c &&
        # sed -i 's/_GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");/\#if defined(__GLIBC__) \&\& !defined(__UCLIBC__) \&\& !__GLIBC_PREREQ(2, 16)\n_GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");\n\#endif/' lib/stdio.in.h  &&
        ./configure
        ''',
        r'intercept-build make -j8'
    ),

    Program(
        r'remind',
        r'https://git.skoll.ca/Skollsoft-Public/Remind/archive/04.00.01.tar.gz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # FIXME: CANNOT COMPILE
    # workman was made for sun solaris systems, not for linux, and I cannot
    # install one of the packages it requires (xview)
    # Program(
    #     r'workman-1.3.4',
    #     r'https://web.mit.edu/kolya/.f/root/net.mit.edu/sipb/user/zacheiss/workman-1.3.4.tar.gz',
    #     r'',
    #     r'',
    #     r''
    # ),

    # requires xaw3dg-dev
    Program(
        r'xfig-3.2.8b',
        r'https://cytranet.dl.sourceforge.net/project/mcj/xfig%2Bfig2dev-3.2.8b.tar.xz',
        r'src',
        r'./configure',
        r'intercept-build make -j8'
    ),

    # FIXME: CANNOT INTERCEPT BUILD SYSTEM
    # zephyr's build system is not easy to intercept.
    # Program(
    #     r'zephyr-main',
    #     r'https://github.com/zephyrproject-rtos/zephyr/archive/refs/heads/main.zip',
    #     r'',
    #     r'',
    #     r'',
    # ),

    # requires: autoconf
    Program(
        r'zsh-5.9',
        r'https://cfhcable.dl.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz',
        r'Src',
        r'./configure',
        r'intercept-build make -j8'
    ),
]
