//2019/1/6 ���̵߳�reactor 
/*
1�� ��Щʵ��Ӧ�õ�ʱ���֣����reactor�����̣߳���ôÿ�����ⲿ����start����������
2�� �����ü�һ���Լ��Ĺ����̣߳�start�����Ժ����߳���Ȼ������ִ�У�
*/
#pragma once
#include "xthreadbase.h"
#include "xReactor.h"
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