find_package (Python3 COMPONENTS Interpreter Development NumPy)

# Python_FOUND
# System has the Python requested components.
# Python_Interpreter_FOUND
# System has the Python interpreter.
# Python_EXECUTABLE
# Path to the Python interpreter.
# Python_INTERPRETER_ID
# A short string unique to the interpreter. Possible values include:
# Python
# ActivePython
# Anaconda
# Canopy
# IronPython
# Python_STDLIB
# Standard platform independent installation directory.
#
# Information returned by distutils.sysconfig.get_python_lib(plat_specific=False,standard_lib=True).
#
# Python_STDARCH
# Standard platform dependent installation directory.
#
# Information returned by distutils.sysconfig.get_python_lib(plat_specific=True,standard_lib=True).
#
# Python_SITELIB
# Third-party platform independent installation directory.
#
# Information returned by distutils.sysconfig.get_python_lib(plat_specific=False,standard_lib=False).
#
# Python_SITEARCH
# Third-party platform dependent installation directory.
#
# Information returned by distutils.sysconfig.get_python_lib(plat_specific=True,standard_lib=False).
#
# Python_Compiler_FOUND
# System has the Python compiler.
# Python_COMPILER
# Path to the Python compiler. Only offered by IronPython.
# Python_COMPILER_ID
# A short string unique to the compiler. Possible values include:
# IronPython
# Python_Development_FOUND
# System has the Python development artifacts.
# Python_INCLUDE_DIRS
# The Python include directories.
# Python_LIBRARIES
# The Python libraries.
# Python_LIBRARY_DIRS
# The Python library directories.
# Python_RUNTIME_LIBRARY_DIRS
# The Python runtime library directories.
# Python_VERSION
# Python version.
# Python_VERSION_MAJOR
# Python major version.
# Python_VERSION_MINOR
# Python minor version.
# Python_VERSION_PATCH
# Python patch version.
# Python_NumPy_FOUND
# System has the NumPy.
# Python_NumPy_INCLUDE_DIRS
# The NumPy include directries.
# Python_NumPy_VERSION
# The NumPy version.

#CHECK: PythonLibs, changed to work in WIN32
if (Python3_Development_FOUND)
set (PYTHON_SOURCES python.c pl2py.c pybips.c py2pl.c pl2pl.c pypreds.c pyio.c)
set (PYTHON_HEADERS py4yap.h)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include_directories( BEFORE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}
${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/os ${CMAKE_SOURCE_DIR}/H ${CMAKE_SOURCE_DIR}/OPTYap  ${CMAKE_SOURCE_DIR}/utf8proc)

#talk to python.pl
add_library(YAPPython  pyload.c ${PYTHON_HEADERS} )

if (WIN32)
add_library (Py4YAP OBJECT ${PYTHON_SOURCES} ${PYTHON_HEADERS})
else()
add_library(Py4YAP SHARED  ${PYTHON_SOURCES} ${PYTHON_HEADERS})

set_target_properties(
    Py4YAP
PROPERTIES VERSION ${YAP_FULL_VERSION}
SOVERSION ${YAP_MAJOR_VERSION}.${YAP_MINOR_VERSION}
)

# arithmetic hassle.      
target_link_libraries(Py4YAP  libYap ${Python3_LIBRARIES})


install(TARGETS  Py4YAP
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
target_link_libraries(YAPPython Py4YAP libYap)


 endif()


# arithmetic hassle.
set_property(TARGET YAPPython PROPERTY CXX_STANDARD_REQUIRED ON)


set_property( SOURCE ${PYTHON_SOURCES}  APPEND PROPERTY COMPILE_DEFINITIONS   YAP_KERNEL=1 _GNU_SOURCE=1)

set (PYTHON_PL python.pl)


add_subDIRECTORY (yap4py)
add_subDIRECTORY (yap_kernel)


install(FILES python.pl DESTINATION ${YAP_INSTALL_DATADIR} )

set_target_properties (YAPPython PROPERTIES PREFIX "")

    install(TARGETS  YAPPython
      LIBRARY DESTINATION ${YAP_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${YAP_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${YAP_INSTALL_LIBDIR} )


endif()
