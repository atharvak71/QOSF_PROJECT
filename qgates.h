#ifndef QGATES_H
#define QGATES_H

#include "qram.h"
#include<sstream>

namespace qcpp
{

static const double s2=0.7071067811865475244008443;

class qhadamard:public qgate
{
public:
	qhadamard():
		qgate(1,"H")
	{}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
	
		out[0]=(in[0]+in[1])*s2;
		out[1]=(in[0]-in[1])*s2;
	}
};
class qnot:public qgate
{
public:
	qnot():qgate(1,"NOT")
	{}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		out[0]=in[1];
		out[1]=in[0];
	}
};
class qcnot:public qgate
{
public:
	qcnot():qgate(2,"CNOT")
	{}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		out[0]=in[0];
		out[1]=in[1];
		out[2]=in[3];
		out[3]=in[2];
	}
};
class qswap:public qgate
{
public:
	qswap():qgate(2,"SWAP")
	{}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		out[0]=in[0];
		out[1]=in[2];
		out[2]=in[1];
		out[3]=in[3];
	}
};
class qcswap:public qgate
{
public:
	qcswap():qgate(3,"CSWAP")
	{
		//0 is control,2,1 are swapped
	}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		//out
		/*2 is control, 0,1 swapped 
		out[0]=in[0];
		out[1]=in[1];
		out[2]=in[2];
		out[3]=in[3];
		out[4]=in[4];
		out[5]=in[6];
		out[6]=in[5];
		out[7]=in[7];
		*/
		
		/*0 is control, 2,1 swapped*/
		out[0]=in[0];
		out[1]=in[1];
		out[2]=in[2];
		out[3]=in[5];
		out[4]=in[4];
		out[5]=in[3];
		out[6]=in[6];
		out[7]=in[7];
	}
};
class qshift:public qgate
{
public:
	qshift(const double& t):
		qgate(1,"shift"),
		a(std::cos(t),std::sin(t))
	{
		std::ostringstream oss("shift-");
		oss << t;
		gate_name=oss.str();
	}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		out[0]=in[0];
		out[1]=a*in[1];
	}
protected:
	std::complex<double> a;
};
}


#endif
