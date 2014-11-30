#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "tbb/task_scheduler_init.h"
namespace GS{

class Global {
public :

     static void Initialize();
     static void Shutdown();
private : 
     Global();
     ~Global();
private: 
     static Global* mpInstance;
private:
    tbb::task_scheduler_init mTbbInit;
};

}

#endif 