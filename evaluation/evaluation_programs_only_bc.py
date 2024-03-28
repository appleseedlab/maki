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
    # requires makeinfo
    Program(
        r'bc-1.07.1',
        r'https://gnu.mirror.constant.com/bc/bc-1.07.1.tar.gz',
        r'bc',
        r'./configure',
        r'intercept-build make -j8'
    )
]
