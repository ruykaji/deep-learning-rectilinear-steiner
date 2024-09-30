from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext
import numpy

ext_modules = [
    Pybind11Extension(
        "connectivity",
        ["connectivity.cpp"],
        include_dirs=[numpy.get_include()],
    ),
]

setup(
    name="connectivity",
    version="0.0.1",
    author="",
    author_email="",
    description="",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
