from setuptools import setup, Extension

setup(
    name='connectivity_module',
    setup_requires=["numpy"],
    install_requires=["numpy"],
    version='0.1',
    ext_modules=[
        Extension('connectivity_module', sources=['lib.c'], include_dirs=["/home/alaie/.local/lib/python3.10/site-packages/numpy/core/include"], )
    ],
)
