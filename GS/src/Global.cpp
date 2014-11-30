#include "Global.h"
#include "adaptive.h"

namespace GS{

Global* Global::mpInstance = NULL;

void Global::Initialize()
{
    if (mpInstance ==NULL)
        mpInstance = new Global();
}

void Global::Shutdown()
{
    if (mpInstance)
        delete mpInstance;
    mpInstance = NULL;
}

Global::Global()
{
    GS::exactinit();
}

    
Global::~Global()
{
}


}

