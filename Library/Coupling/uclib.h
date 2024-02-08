#include <iostream>
//#include <filesystem>

#ifndef UCLIB_H
#define UCLIB_H
#ifdef DOUBLE_PRECISION
typedef double Real;
#else
typedef float Real;
#endif
typedef double CoordReal;

#ifdef __cplusplus
extern "C" {
#endif
#if defined(WIN32) || defined(_WINDOWS) || defined(_WINNT)
# define USERFUNCTION_EXPORT __declspec(dllexport)
# define USERFUNCTION_IMPORT __declspec(dllimport)
#else
# define USERFUNCTION_EXPORT
# define USERFUNCTION_IMPORT
#endif

extern void USERFUNCTION_IMPORT ucarg(void *, const char *, const char *, int);
extern void USERFUNCTION_IMPORT ucfunc(void * , const char *, const char *);
extern void USERFUNCTION_IMPORT ucfunction(void *, const char *, const char *, int, ...);

void USERFUNCTION_EXPORT uclib();
#ifdef __cplusplus
}
#endif
#endif
