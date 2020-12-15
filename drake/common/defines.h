#pragma once

#ifndef WIN32
#  define DRAKE_API
#else
#  ifdef drake_EXPORTS
#    define DRAKE_API __declspec(dllexport)
#  else
#    define DRAKE_API __declspec(dllimport)
#  endif
#endif