#pragma once

#define SAFE_RELEASE(ptr) if (ptr) {delete ptr; ptr = 0;}
#ifndef _DEBUG
#define _USE_MATH_DEFINES
#endif

