#include "QRegister.h"
#include "QMachine.h"
#include "StandardGates.h"

QOperationRecord invert(QRegister& r)
{
	return (r^=~0);
}



QOperationRecord QRegister::operator+=(const int id)		//Not necessary
{
	static ADDI addg(parent_machine);
	return addg({*this},id);
}
QOperationRecord QRegister::operator-=(const int id)		//Not necessary
{
	static ADDI subg(parent_machine);
	return subg({*this},-id);
}
QOperationRecord QRegister::operator<<=(const size_t ls)		//Not necessary
{
	static CIRCRSHIFTI rshiftg(parent_machine);
	return rshiftg({*this},-(int)ls);
}
QOperationRecord QRegister::operator>>=(const size_t ls)		//Not necessary
{
	static CIRCRSHIFTI rshiftg(parent_machine);
	return rshiftg({*this},(int)ls);
}
QOperationRecord QRegister::operator^=(const size_t ls)		//Not necessary
{
	static XORI xorg(parent_machine);
	return xorg({*this},ls);
}

/*	
	QOperationRecord operator+=(const QRegister&);	//Not necessary
	QOperationRecord operator-=(const QRegister&);
	QOperationRecord operator<<=(const QRegister&); 	//CIRCULAR shifts because operations must be reversible
	QOperationRecord operator>>=(const QRegister&); 
 	QOperationRecord operator^=(const QRegister&);
	
	QOperationRecord operator!=();
	
private:
	//QRegister& operator=	//Registers can't be copied.  They are really references to machine ram..but the reliance on the machine ensures this
};
std::ostream& operator<<(std::ostream& out,QRegister& mac);
*/
	
//((input >> permutation.indices[i]) & 0x1) << i;
	
//this is output[i]=input[index[i]]
//{0,1,2,3,4,5,6,7,8,9,10,11,12}
//
permutation_t permutation_t::inverse() const
{
	permutation_t pt;
	for(cl_long i=0;i<(REG_MAX_SIZE);i++)
	{
		pt.indices[indices[i]]=i;
	}
	return pt;
}

permutation_t::permutation_t()
{
	for(cl_long i=0;i<(REG_MAX_SIZE);i++)
	{
		indices[i]=i;
	}
}
permutation_t::permutation_t(const std::initializer_list<char>& inp)
{
	bool used[REG_MAX_SIZE];
	std::fill(used,used+REG_MAX_SIZE,false);
	char *lst=indices;
	for(std::initializer_list<char>::const_iterator ci=inp.begin();ci!=inp.end() && (lst < indices+REG_MAX_SIZE);++ci)
	{
		cl_char d=*ci & 0x3F;
		used[d]=true;
		*lst++=d;
	}
	for(int i=0;i<REG_MAX_SIZE;i++)
	{
		if(!used[i])
		{
			*lst++=i;
		}
	}
}

std::ostream& operator<<(std::ostream& out,const permutation_t& p)
{
	out << "{";
	for(int i=0;i<REG_MAX_SIZE;i++)
	{
		out << (int)p.indices[i] << ",";
	}
	out << "}";
	return out;
}
