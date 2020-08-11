from setuptools import setup

setup(
    name="cadump",
    version="0.0.12",
    author="Paul Scherrer Institute",
    author_email="daq@psi.ch",
    description="Interface to dump data from archiver/databuffer",
    packages=["cadump"],
    entry_points={
        'console_scripts': [
            'cadump_server = cadump.cadump:main',
        ],
    }
)
