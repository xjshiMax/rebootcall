//2018-12-18
//xheaptimer 作为定时器使用方法，
//1）定义xheaptimer对象
//2）定义xreactor，然后注册定时事件，启动xreactor
//3）该定时器只作用一次。
#pragma once
#include<iostream>
#ifdef WIN32
#include <time.h>
#else
#include <netinet/in.h>
#include <time.h>
#endif
namespace SAEBASE{
#define BUFFER_SIZE 64


//使用小根对来实现定时器
class xheaptimer
{
public:
	//延时时间，单位秒s.
	xheaptimer(int delay)
	{
		expire = time(NULL)+delay;
	}
public:
	time_t expire;
	void (*cb_func)(void*);
	void* user_data;
};

//小根堆的实现
class xtime_heap
{
public:
	xtime_heap(int cap) throw(std::exception)
		:m_capacity(cap),m_cur_size(0)
	{
		array = new xheaptimer*[m_capacity];
		if(!array)
		{
			throw std::exception();
		}
		for(int i=0;i<m_capacity;i++)
		{
			array[i]=NULL;
		}
	}
	xtime_heap(xheaptimer** init_array,int size,int capacity) throw(std::exception)
		:m_cur_size(size),m_capacity(capacity)
	{
		if(m_capacity < size)  //超过堆的最大容量，抛异常
		{
			throw std::exception();
		}
		array = new xheaptimer*[m_capacity];
		if(!array) throw std::exception();
		for(int i=0;i<m_capacity;i++)
		{
			array[i]=NULL;
		}
		if(size!=0)
		{
			for(int i=1;i<size;i++)
			{
				array[i]=init_array[i];
			}
			for(int i=(m_cur_size)/2;i>=0;--i)
			{
				percolate_down(i);
			}
		}
	}
	virtual ~xtime_heap()
	{

	}
public:
	void add_timer(xheaptimer*timer)throw(std::exception);
	void del_timer(xheaptimer* timer);
	xheaptimer * top()const ;
	void pop_timer();
	void tick();
	bool empty()const {return m_cur_size==0;}
private:
	void percolate_down(int hole);  //最小堆调整
	void resize()throw(std::exception);



private:
	xheaptimer**array;
	int m_capacity;
	int m_cur_size;
};
void xtime_heap::percolate_down(int hole)
{
	xheaptimer* temp = array[hole];
	int child=0;
	for(;((hole*2+1)<=(m_cur_size-1));hole=child)
	{
		child=hole*2+1;
		if(child<(m_cur_size-1)&& (array[child+1]->expire < array[child]->expire))
		{
			child++; //子节点中较小的元素
		}
		if(array[child]->expire < temp->expire)
		{
			array[hole] = array[child]; //如果较小的子节点小于根节点，则上旋。
		}
		else
		{
			break;
		}
		//array[hole] = temp;

	}
	array[hole] = temp;// 在做小根对调整时，最后赋值。
}
void xtime_heap::resize() throw (std::exception)
{
	xheaptimer**temp = new xheaptimer*[2*m_capacity];
	for(int i=0;i<2*m_capacity;++i)
	{
		temp[i]=NULL;
	}
	if(!temp)throw std::exception();
	int temp_cap=2*m_capacity;
	for (int i=0;i<m_cur_size;++i)
	{
		temp[i] = array[i];
	}
	delete [] array;
	array=temp;
}
void xtime_heap::add_timer(xheaptimer*timer)throw(std::exception)
{
	if(!timer)
	{
		return ;
	}
	if(m_cur_size >= m_capacity)
	{
		resize();
	}
	int hole=m_cur_size++;
	int parent=0;
	//将新加入的元素放在最后面，然后上旋调整。
	for(;hole>0;hole=parent)
	{
		parent = (hole-1)/2;
		if(array[parent]->expire <= timer->expire)
		{
			break;
		}
		array[hole]=array[parent];
	}
	array[hole]=timer;
}
void xtime_heap::del_timer(xheaptimer* timer)
{
	if(!timer)
	{
		return;
	}
	//这里将定时器的相应接口给置为空，还没有从队列删除
	timer->cb_func = NULL;
	//pop_timer()
}
xheaptimer * xtime_heap::top()const 
{
	if(empty())
		return NULL;
	return array[0];
}
void xtime_heap::pop_timer()
{
	if(empty())
		return;
	if(array[0])
	{
		//将最后面的元素放在第一位，然后向下调整小根堆
		//delete array[0];
		//在出队列的时候，直接将指针指向别的地方，不做删除。如果是对象，则结束自动释放
		//如果是new的内存，由外部释放。
		array[0]=array[--m_cur_size];
		percolate_down(0);
		array[m_cur_size]=NULL;
	}
}
void xtime_heap::tick()
{
	xheaptimer* temp=array[0];
	time_t cur = time(NULL);
	while(!empty())
	{
		if(!temp)
		{
			break;
		}
		if(temp->expire >cur)
			break;
		if(array[0]->cb_func)
		{
			array[0]->cb_func(array[0]->user_data);
		}
		pop_timer();
		temp=array[0];
	}
}
}