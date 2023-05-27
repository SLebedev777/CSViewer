cmake_minimum_required (VERSION 3.10)

project(libiconv)

string(TOUPPER ${PROJECT_NAME} PRJ)

if (MSVC)
	set(LIBICONV_TEST_MESSAGE "Making ${PROJECT_NAME} for MSVC")
else()
	set(LIBICONV_TEST_MESSAGE "Making ${PROJECT_NAME}")
endif()
message("${LIBICONV_TEST_MESSAGE}")

set(LIBICONV_SRC ${PROJECT_SOURCE_DIR}/src)
set(LIBICONV_INCLUDE ${PROJECT_SOURCE_DIR}/include)
message("${LIBICONV_SRC}")

# adjust defines for libiconv
set (ICONV_CONST "const")
set (HAVE_WCHAR_H 1)
set (HAVE_WCHAR_T 1)
set (HAVE_MBRTOWC 1)
set (HAVE_WCRTOMB 1)
set (HAVE_SETLOCALE 1)
set (WORDS_LITTLEENDIAN 1)
set (BROKEN_WCHAR_H 0)
set (USE_MBSTATE_T 1) 

# make includes
if (MSVC)
    file (WRITE ${LIBICONV_SRC}/include_win/config.h.in
        "#cmakedefine ICONV_CONST @ICONV_CONST@\n"
        "#cmakedefine HAVE_WCHAR_H @HAVE_WCHAR_H@\n"
        "#cmakedefine HAVE_WCHAR_T @HAVE_WCHAR_T@\n"
        "#cmakedefine HAVE_MBRTOWC @HAVE_MBRTOWC@\n"
        "#cmakedefine HAVE_WCRTOMB @HAVE_WCRTOMB@\n"
        "#cmakedefine HAVE_SETLOCALE @HAVE_SETLOCALE@\n"
        "#cmakedefine WORDS_LITTLEENDIAN @WORDS_LITTLEENDIAN@\n"
        "#cmakedefine BROKEN_WCHAR_H @BROKEN_WCHAR_H@\n"
        "#cmakedefine USE_MBSTATE_T @USE_MBSTATE_T@\n"
    )
    configure_file (
        ${LIBICONV_SRC}/include_win/config.h.in
        ${LIBICONV_SRC}/include_win/config.h
) 
endif()
configure_file(${LIBICONV_SRC}/include/iconv.h.in
				${LIBICONV_INCLUDE}/iconv.h)
configure_file(${LIBICONV_SRC}/libcharset/include/libcharset.h.in
				${LIBICONV_INCLUDE}/libcharset.h)
configure_file(${LIBICONV_SRC}/libcharset/include/localcharset.h.in
				${LIBICONV_INCLUDE}/localcharset.h)


add_compile_definitions(LIBCHARSET_DLL_EXPORTED= set_relocation_prefix=libcharset_set_relocation_prefix relocate=libcharset_relocate HAVE_CONFIG_H
INSTALLPREFIX=NULL NO_XMALLOC BUILDING_LIBCHARSET ENABLE_RELOCATABLE=1 IN_LIBRARY)


# libcharset
set (SRC_LIBCHARSET
  ${LIBICONV_SRC}/libcharset/lib/localcharset.c
  ${LIBICONV_SRC}/libcharset/lib/relocatable-stub.c
)

# iconv
set(SRC_LIBICONV)
list(APPEND SRC_LIBICONV
	${LIBICONV_SRC}/lib/iconv.c
    ${LIBICONV_SRC}/lib/relocatable.c
)

add_library (${PROJECT_NAME} STATIC ${SRC_LIBICONV} ${SRC_LIBCHARSET})

target_include_directories (${PROJECT_NAME}
  PRIVATE
    ${LIBICONV_SRC}/include_win
  PUBLIC
    ${LIBICONV_INCLUDE}
  ) 

target_compile_options (${PROJECT_NAME} 
  PUBLIC
    -DLIBCHARSET_STATIC 
  PRIVATE 
    -DLIBDIR="./lib"
    -DIN_LIBRARY -DNO_XMALLOC -DHAVE_CONFIG_H
    -DENABLE_RELOCATABLE=1 -Dset_relocation_prefix=libiconv_set_relocation_prefix -Drelocate=libiconv_relocate 
  ) 
