#include<iostream>
#include "../qram.h"
#include "../qgates.h"
#include<cstdlib>
using namespace qcpp;
using namespace std;


class Uf:public qgate
{
public:
	enum Fchoice
	{
		ZERO=0,
		ONE,
		NOT,
		IDENTITY
	};
	Uf(const Fchoice& f):qgate(2,"Uf"),
			fc(f)
	{
	}

	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		for (int i=0;i<4;i++)
		{
			int x=i & 1;
			int y=(i & 2) >> 1;
			int ni=0;
			int fx;

			switch (fc)
			{
				case ZERO:
					fx=0;break;
				case ONE:
					fx=1;break;
				case IDENTITY:
					fx=x;break;
				case NOT:
					fx=(~x) & 1;break;
			};

			ni=((fx ^ y) << 1)+x;

			out[ni]=in[i];
		};
	}

protected:
	Fchoice fc;
};

int main(int argc,char**)
{
	const int xbit=1;
	const int ybit=2;

	srand(time(0));

	Uf::Fchoice gate_select=(Uf::Fchoice)(rand() & 3);//chose a random implementation of F

	qram r(2,std::clog);

	r.op(qnot(),ybit);
	r.op(qhadamard(),xbit);
	r.op(qhadamard(),ybit);
	r.op(Uf(gate_select),xbit | ybit);
	r.op(qhadamard(),xbit);
	r.op(qhadamard(),ybit);

	cout << "\nMeasuring the x bit gives:" << endl;

	qram::measurement m=r.measure(xbit);

	cout << m << endl;

	if (m==0)
		cout << "Therefore, the function is a constant function" << endl;
	else
		cout << "Therefore, the function is NOT a constant function" << endl;

	return 0;

}
