#pragma once

#define SAFE_RELEASE(ptr) if (ptr) {delete ptr; ptr = 0;}
#define SAFE_RELEASE_ARRAY(ptr) if (ptr) {delete	[] ptr; ptr = 0;}

#ifndef _DEBUG
#define _USE_MATH_DEFINES
#define NDEBUG
#else
extern int countd1, countd2, countd3, countd4, countd5;
#endif

