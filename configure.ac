#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
#AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_INIT([zunda], [2.0b4], [jmizuno@gmail.com])
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
						[boost-include],
						[AC_HELP_STRING([--with-boost-include=DIR], [boost include directory])],
						[AM_CPPFLAGS="-I${withval} ${AM_CPPFLAGS}"]
						)
AC_ARG_WITH(
						[boost-lib],
						[AC_HELP_STRING([--with-boost-lib=DIR], [boost library directory])],
						[AM_LDFLAGS="-L${withval} ${AM_LDFLAGS}"]
						)
BOOST_LIBS="-lboost_system -lboost_filesystem -lboost_program_options -lboost_regex"
AC_SUBST(BOOST_LIBS)
#old_CPPFLAGS=${CPPFLAGS}
#old_LDFLAGS=${LDFLAGS}
#CPPFLAGS=${AM_CPPFLAGS}
#LDFLAGS=${AM_LDFLAGS}
#AC_CHECK_HEADERS([boost/algorithm/string.hpp],,[AC_MSG_ERROR(boost/algorithm/string.hpp not found)])
#AC_CHECK_HEADERS([boost/date_time/posix_time/posix_time.hpp],   ,[AC_MSG_ERROR(boost/date_time/posix_time/posix_time.hpp not found)])
#AC_CHECK_HEADERS([boost/filesystem.hpp],      ,[AC_MSG_ERROR(boost/filesystem.hpp not found)])
#AC_CHECK_HEADERS([boost/foreach.hpp],         ,[AC_MSG_ERROR(boost/foreach.hpp not found)])
#AC_CHECK_HEADERS([boost/format.hpp],          ,[AC_MSG_ERROR(boost/format.hpp not found)])
#AC_CHECK_HEADERS([boost/lexical_cast.hpp],    ,[AC_MSG_ERROR(boost/lexical_cast.hpp not found)])
#AC_CHECK_HEADERS([boost/program_options.hpp], ,[AC_MSG_ERROR(boost/program_options.hpp not found)])
#AC_CHECK_HEADERS([boost/regex.hpp],           ,[AC_MSG_ERROR(boost/regex.hpp not found)])
#AC_CHECK_HEADERS([boost/unordered_map.hpp],   ,[AC_MSG_ERROR(boost/unordered_map.hpp not found)])
#AC_CHECK_LIB([boost_regex], main,          ,[AC_MSG_ERROR(boost_regex library not found)])
#AC_CHECK_LIB([boost_program_options], main,,[AC_MSG_ERROR(boost_program_options library not found)])
#AC_CHECK_LIB([boost_system], main,         ,[AC_MSG_ERROR(boost_system library not found)])
#LDFLAGS="-lboost_system ${AM_LDFLAGS}"
#AC_CHECK_LIB([boost_filesystem], main,     ,[AC_MSG_ERROR(boost_filesystem library not found)])
#CPPFLAGS=${old_CPPFLAGS}
#LDFLAGS=${old_LDFLAGS}

AC_ARG_WITH(
						[cabocha],	
						[AC_HELP_STRING([--with-cabocha=DIR], [cabocha directory])],
						[AM_CPPFLAGS="-I${withval}/include ${AM_CPPFLAGS}"; AM_LDFLAGS="-L${withval}/lib ${AM_LDFLAGS}"]
						)
AC_ARG_ENABLE(
							[cabocha],
							AC_HELP_STRING([--disable-cabocha], [do not use cabocha (default is no)]),
							[enable_cabocha=no],
							)

AS_IF([test "x$enable_cabocha" != xno],
			[AM_CXXFLAGS=" -DUSE_CABOCHA ${AM_CXXFLAGS}"
			 old_CPPFLAGS=${CPPFLAGS}
			 old_LDFLAGS=${LDFLAGS}
			 CPPFLAGS=${AM_CPPFLAGS}
			 LDFLAGS=${AM_LDFLAGS}
			 AC_CHECK_HEADER([cabocha.h], [break]  ,[AC_MSG_ERROR(cabocha.h not found)])
			 AC_CHECK_LIB([cabocha], main,   ,[AC_MSG_ERROR(cabocha library not found)])
			 CPPFLAGS=${old_CPPFLAGS}
			 LDFLAGS=${old_LDFLAGS}
			 ]
			)

AC_ARG_WITH(
						[crfsuite],
						[AC_HELP_STRING([--with-crfsuite=DIR], [crfsuite directory])],
						[AM_CPPFLAGS="-I${withval}/include ${AM_CPPFLAGS}"; AM_LDFLAGS="-L${withval}/lib ${AM_LDFLAGS}"]
						)

AC_ARG_WITH(
						[crfpp],
						[AC_HELP_STRING([--with-crfpp=DIR], [crfpp directory])],
						[AM_CPPFLAGS="-I${withval}/include ${AM_CPPFLAGS}"; AM_LDFLAGS="-L${withval}/lib ${AM_LDFLAGS}"]
						)

CRF_TYPE=crfsuite
AC_ARG_WITH(
						[crf],
						[AC_HELP_STRING([--with-crf=crf], [set crf (crfsuite|crfpp|none) default: crfsuite])],
						[CRF_TYPE="${withval}"]
						)

case "$CRF_TYPE" in
	none)
		AC_MSG_WARN([CRF is DISABLED, functional expression analysis is also disabled. The performance of modality analysis is limited.])
		;;
	crfsuite)
		AM_CXXFLAGS=" -DUSE_CRFSUITE ${AM_CXXFLAGS}"
		old_CPPFLAGS=${CPPFLAGS}
		old_LDFLAGS=${LDFLAGS}
		CPPFLAGS=${AM_CPPFLAGS}
		LDFLAGS=${AM_LDFLAGS}
		AC_CHECK_HEADER([crfsuite.hpp], [break]  ,[AC_MSG_ERROR(crfsuite.hpp not found)])
		AC_CHECK_LIB([crfsuite], main,   ,[AC_MSG_ERROR(crfsuite library not found)])
		CPPFLAGS=${old_CPPFLAGS}
		LDFLAGS=${old_LDFLAGS}
		;;
	crfpp)
		AM_CXXFLAGS=" -DUSE_CRFPP ${AM_CXXFLAGS}"
		old_CPPFLAGS=${CPPFLAGS}
		old_LDFLAGS=${LDFLAGS}
		CPPFLAGS=${AM_CPPFLAGS}
		LDFLAGS=${AM_LDFLAGS}
		AC_CHECK_HEADER([crfpp.h], [break]  ,[AC_MSG_ERROR(crfpp.h not found)])
		AC_CHECK_LIB([crfpp], main,   ,[AC_MSG_ERROR(crfpp library not found)])
		CPPFLAGS=${old_CPPFLAGS}
		LDFLAGS=${old_LDFLAGS}
		;;
	*)
		AC_MSG_ERROR([--with-crf takes crfpp, crfsuite or none])
		;;
esac



AC_ARG_ENABLE(
							[debug],
							AC_HELP_STRING([--enable-debug], [debug mode (default is no)]),
							[ AM_CXXFLAGS="-D_MODEBUG -Werror -O0 -fno-inline -g -pg ${AM_LDFLAGS} ${AM_CXXFLAGS}"],
							)

DICDIR=${datarootdir}/${PACKAGE}/dic
AC_SUBST(DICDIR)
MODELDIR_IPA=${datarootdir}/${PACKAGE}/model_ipa
AC_SUBST(MODELDIR_IPA)
MODELDIR_JUMAN=${datarootdir}/${PACKAGE}/model_juman
AC_SUBST(MODELDIR_JUMAN)
AM_CXXFLAGS="-DMODELDIR_IPA=\\\"${MODELDIR_IPA}\\\" -DMODELDIR_JUMAN=\\\"${MODELDIR_JUMAN}\\\" -DDICDIR=\\\"${DICDIR}\\\" ${AM_CXXFLAGS}"

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(AM_CXXFLAGS)

AC_CONFIG_FILES([Makefile src/Makefile dic/Makefile model_ipa/Makefile model_juman/Makefile liblinear-1.8/Makefile liblinear-1.8/blas/Makefile cdbpp-1.1/Makefile cdbpp-1.1/include/Makefile tinyxml2/Makefile])
AC_OUTPUT



