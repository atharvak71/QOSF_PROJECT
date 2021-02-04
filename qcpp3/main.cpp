#include <iostream>
#include "QMachine.h"
#include "QGate.h"
#include "StandardGates.h"
#include<iterator>
using namespace std;

#include<iostream>

int main()
{
	try
	{
		QMachine qm(4);
		
		
		//bad api, should really have a qreg<N> type
		
		
		QRegister r1(qm,{0,1});
		QRegister r2(qm,{2,3});
		
		r1+=2;
		r2+=1;

		MEASUREMENT mgate(qm);
		
//		r1+=r2;
//		r1<<=3;
//		hadamard(r2);
//		r1-=1;
		
		
	/*	MEASUREMENT mgate(qm,1);
		HADAMARD hgate(qm);
		
		hgate();
		hgate({1});
		
		mgate();
		
		cerr << "result " << mgate.result << endl;*/
	}
	catch(const cl::Error& e)
	{
		std::cerr << e.what() << "(" << e.err() << ")" << std::endl;
	}
	return 0;
}
