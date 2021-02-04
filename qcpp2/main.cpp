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
		
		MEASUREMENT mgate(qm,1);
		HADAMARD hgate(qm);
		
		hgate();
		hgate({1});
		
		mgate();
		
		cerr << "result " << mgate.result << endl;
	}
	catch(const cl::Error& e)
	{
		std::cerr << e.what() << "(" << e.err() << ")" << std::endl;
	}
	return 0;
}
