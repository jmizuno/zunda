#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
#AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_INIT([zunda], [1.0], [jmizuno@gmail.com])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS(iostream,,[AC_MSG_ERROR(iostream not found)])
AC_CHECK_HEADERS(fstream, ,[AC_MSG_ERROR(fstream not found)])
AC_CHECK_HEADERS(cstdlib, ,[AC_MSG_ERROR(cstdlib not found)])
AC_CHECK_HEADERS(vector,  ,[AC_MSG_ERROR(vector not found)])
AC_CHECK_HEADERS(sstream, ,[AC_MSG_ERROR(sstream not found)])
AC_CHECK_HEADERS(string,  ,[AC_MSG_ERROR(string not found)])
AC_CHECK_HEADERS(iomanip, ,[AC_MSG_ERROR(iomanip not found)])

AC_ARG_WITH(
						boost,
						[AC_HELP_STRING([--with-boost=DIR],[boost directory])],
						[INCLUDES=${withval}/include ${INCLUDES}; LDFLAGS=${withval}/lib ${LDFLAGS}]
						)
AC_CHECK_HEADERS(boost/algorithm/string.hpp,,[AC_MSG_ERROR(boost/algorithm/string.hpp not found)])
AC_CHECK_HEADERS(boost/date_time/posix_time/posix_time.hpp,   ,[AC_MSG_ERROR(boost/date_time/posix_time/posix_time.hpp not found)])
AC_CHECK_HEADERS(boost/filesystem.hpp,      ,[AC_MSG_ERROR(boost/filesystem.hpp not found)])
AC_CHECK_HEADERS(boost/foreach.hpp,         ,[AC_MSG_ERROR(boost/foreach.hpp not found)])
AC_CHECK_HEADERS(boost/format.hpp,          ,[AC_MSG_ERROR(boost/format.hpp not found)])
AC_CHECK_HEADERS(boost/lexical_cast.hpp,    ,[AC_MSG_ERROR(boost/lexical_cast.hpp not found)])
AC_CHECK_HEADERS(boost/program_options.hpp, ,[AC_MSG_ERROR(boost/program_options.hpp not found)])
AC_CHECK_HEADERS(boost/regex.hpp,           ,[AC_MSG_ERROR(boost/regex.hpp not found)])
AC_CHECK_HEADERS(boost/unordered_map.hpp,   ,[AC_MSG_ERROR(boost/unordered_map.hpp not found)])
AC_CHECK_LIB(boost_regex, main,          ,[AC_MSG_ERROR(boost_regex library not found)])
AC_CHECK_LIB(boost_program_options, main,,[AC_MSG_ERROR(boost_program_options library not found)])
AC_CHECK_LIB(boost_filesystem, main,     ,[AC_MSG_ERROR(boost_filesystem library not found)])
AC_CHECK_LIB(boost_system, main,         ,[AC_MSG_ERROR(boost_system library not found)])

AC_ARG_WITH(
						cabocha,	
						[AC_HELP_STRING([--with-cabocha=DIR], [cabocha directory])],
						[INCLUDES=${withval}/include ${INCLUDES}; LDFALGS=${LDFALGS}]
						)
AC_CHECK_HEADERS(cabocha.h,   ,[AC_MSG_ERROR(cabocha.h not found)])
AC_CHECK_LIB(cabocha, main,   ,[AC_MSG_ERROR(cabocha library not found)])

AC_ARG_ENABLE(
							[debug],
							AC_HELP_STRING([--enable-debug], [debug mode (default is no)]),
							[ CXXFLAGS="-D_MODEBUG -Werror -O0"; LDFLAGS="-fno-inline -g -pg ${LDFLAGS}";],
							)

AC_SUBST(INCLUDES)
AC_SUBST(LDFALGS)

DICDIR=${datarootdir}/${PACKAGE}/dic
AC_SUBST(DICDIR)

MODELDIR_IPA=${datarootdir}/${PACKAGE}/model_ipa
MODELDIR_JUMAN=${datarootdir}/${PACKAGE}/model_juman
AC_SUBST(MODELDIR_IPA)
AC_SUBST(MODELDIR_JUMAN)

AC_CONFIG_FILES([Makefile src/Makefile dic/Makefile model_ipa/Makefile model_juman/Makefile liblinear-1.8/Makefile liblinear-1.8/blas/Makefile cdbpp-1.0/Makefile tinyxml2/Makefile])
AC_OUTPUT



