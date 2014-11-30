// GSTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <stack>
#include "VecTest.h"
#include "transform.h"
#include "Box3.h"
#include "Box2.h"
#include "Surface.h"
#include "arithmetic.h"
#include "typedefs.h"

int  OperatorPriority(int c);
bool  IsOperator(char c);
std::string  InfixToPostfix(const std::string& infix)
{
    std::string postfix ;
    std::stack<char> opstack;
    for (int i = 0; i< infix.size(); i++)
    {
        if (!IsOperator(infix[i]))
        {
            postfix +=infix[i];
            continue;
        } 
        postfix+=' ';
         if (opstack.empty() || (opstack.top() == '(' && infix[i] !=')'))
            opstack.push(infix[i]);
        else {
            while (OperatorPriority(infix[i]) <= OperatorPriority(opstack.top()))
            {
               if(opstack.top() != '(')
               {
                   
                    postfix +=opstack.top();
                    opstack.pop();
               }else
                    break;
            }
            if (infix[i] != ')')
                opstack.push(infix[i]);
            else if (opstack.top() == '(')
                 opstack.pop();
        }
    }
    while (!opstack.empty())
    {
        postfix+= opstack.top();
        opstack.pop();
    }
    return postfix;
}

bool  IsOperator(char c)
{
    if ((c == '(') || (c == ')') ||(c == '+')
          || (c == '*') || (c == '-'))
          return true;
    return false ; 
}

int  OperatorPriority(int c)
{
    if( c== ')')
        return 0;
    if ((c == '+') || (c == '-'))
        return 1;
    if (c == '*')
        return 2;
    return 3;// '('
}



int _tmain(int argc, _TCHAR* argv[])
{  

       std::string infix ="(03+1)*2";
      std::string s = InfixToPostfix(infix);

        double f1 = 1.0000000000000004;
        double f2 = 0.9999999999999999;
        bool same = GS::double_compare(f1, f2);
        bool samefloat = GS::float_compare(f1, f2);
        bool bSame = f1 == f2;
	    GS::mat2<float> mat;
		mat[0].x = 2;
		mat[0].y = 1;
	 	mat[1].x = 0;
		mat[1].y = 3;
		GS::float2 p(1,2);
		GS::float2 r=GS::mul(mat, p);
		std::hash_map<GS::IndexPair, bool,GS::IndexPairCompare>   SegIndexHash;
		GS::IndexPair index;
		index.ID[0] = 100;
		index.ID[1] = 1380;
		SegIndexHash[index] =true;
		index.ID[0] = 15689;
		index.ID[1] = 1732;
		SegIndexHash[index] =true;

        bool result = SegIndexHash[index];


	GS::Box3 box(10, 10, 10, 100, 100, 100);
	GS::Box2 box2(10, 10, 100, 100);
	GS::float2 size = box2.Diagonal();
	GS::float3 length = box.Diagonal();
	Vec2Test();
	Vec3Test();
	Vec4Test();
	return 0;
}

