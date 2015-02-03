#pragma once

#define SAFE_RELEASE(ptr) if (ptr) {delete ptr; ptr = 0;}

typedef unsigned uint;

