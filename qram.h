#ifndef QRAM_H
#define QRAM_H
#include<cstddef>
#include<complex>
#include<iosfwd>
#include<vector>
#include<cmath>

namespace qcpp
{
	class qgate;
	class qram;
}

std::ostream& operator<<(std::ostream&,const qcpp::qram&);


namespace qcpp
{
class qgate
{
public:
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const=0;
	const unsigned int gate_bits;
	std::string gate_name;
protected:
	qgate(const unsigned int gb,const std::string& gn);
};

class qram
{
public:
	struct measurement
	{
		measurement(std::size_t sz):state(sz) {}
		bool check_if(std::size_t reg) const
		{
			return std::abs(std::abs(state[reg])-1.0) < 10e-7; 
		}
		bool operator==(const std::size_t& r) const
		{
			return check_if(r);
		}
		
		std::vector<std::complex<double> > state;
		unsigned num_bits;
	};
	void permute_internal_down(std::size_t mask);
	void permute_internal_up(std::size_t mask);
	
	void op(const qgate& g,std::size_t mask);
	measurement measure(std::size_t mask) const;
	
	qram(const unsigned int& num_bits,std::ostream& os,const unsigned int& initialstate=0);
	~qram();
		
	std::complex<double>* state;
	
	
	const std::size_t num_entries;
	const unsigned int num_bits;
protected:
		
	
	std::ostream& oss;
	std::complex<double>* state_back;
	friend std::ostream& ::operator<<(std::ostream&,const qram&);
};
}

std::ostream& operator<<(std::ostream&,const qcpp::qram::measurement&);

#include "qram.inl"

#endif
