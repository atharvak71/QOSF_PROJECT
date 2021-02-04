#include<qram.h>
#include<qgates.h>
using namespace std;

int main(int,char**)
{
    qram r(3,std::log);
    r.op(qnot(),0x2);
    r.op(qcnot(),0x5);
    r.op(qtoffoli(),0x7)
    r.op(qcrotate(theta),0x5);
    qram::measurement m;
    m=r.measure(0x6);
    cout << m;

    return 0;
}
