//2019/1/6 ���̵߳�reactor 
/*
1�� ��Щʵ��Ӧ�õ�ʱ���֣����reactor�����̣߳���ôÿ�����ⲿ����start����������
2�� �����ü�һ���Լ��Ĺ����̣߳�start�����Ժ����߳���Ȼ������ִ�У�
*/
#include "xthreadbase.hpp"
#include "xReactor.hpp"
namespace SAEBASE{
class xReactorwithThread:public xReactor,public Threadbase
{
public:
	virtual void run()
	{
		xReactor::start();
	}
	void startReactorWithThread()
	{
		Threadbase::start();
	}
};
}