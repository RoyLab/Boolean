#pragma once

namespace GS{

template< typename T> class AutoPtr{
   struct SharedData {
       T* mptr;
       int ref;
    SharedData()
        : mptr(nullptr)
        , ref(0)
       {
       }
    ~SharedData ()
    {
        if (mptr)
            delete mptr;
        mptr= nullptr;
    }
   };
public: 
   
    AutoPtr(T* ptr = 0 )
    {
        mpData = new SharedData;
        mpData->mptr = ptr;
        mpData->ref++;
    }

    AutoPtr(AutoPtr<T>& obj)
    {
        
        obj.mpData->ref++;
        mpData = obj.mpData;

    }
    
    AutoPtr<T>& operator = (AutoPtr<T>& other)
    {
        if (mpData == other.mpData)
            return *this ;
        Release();
        mpData = other.mpData;
        mpData->ref++;
        return *this;
    }

     AutoPtr<T>& operator = (T* other)
    {
        if (mpData->mptr == other)
            return *this ;
        if (mpData->mptr)
        {
            Release();
            mpData = new SharedData;
            mpData->mptr = other;
            mpData->ref++;
        }else 
            mpData->mptr = other;
        return *this;
    }
    
    inline T* operator ->() const 
    {
        return mpData->mptr;
    }

    inline T& operator *() const 
    {
        return *(mpData->mptr);
    }
    bool operator==(T* other) const
    {
        return mpData->mptr == other;
    }
    void  Release()
    {
         mpData->ref --;
        if (mpData->ref  ==0 )
        {
            delete mpData;
            mpData = nullptr;
        }

    }

    inline int use_count() const {return mpData->ref;}
    ~AutoPtr()
    {
        Release();
    }
private:
    SharedData* mpData; 
};

}