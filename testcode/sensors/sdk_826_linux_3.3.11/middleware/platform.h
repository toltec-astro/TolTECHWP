#ifndef _INC_PLATFORM_H_
#define _INC_PLATFORM_H_

#ifdef RT_SUPPORT
#define _LINUX
#endif
#ifndef _LINUX  // Windows --------------------------------------------

#ifdef S826_EXPORTS
#ifdef _WIN32
#define S826_API __declspec(dllexport)
#endif
#else
#define S826_API __declspec(dllimport)
#endif

// calling convention
#define S826_CC __stdcall


#else // Linux --------------------------------------------------------

#define S826_API
#define S826_CC

#endif // -------------------------------------------------------------

// TODO: Why are some of these types defined here and also in
//       826drv.h(uint) and 826api.h
typedef unsigned int        uint;
typedef unsigned short      uint16;
typedef short               int16;

#ifndef NULL
#define NULL    ((void *)0)
#endif


#endif // ifndef _INC_PLATFORM_H_
