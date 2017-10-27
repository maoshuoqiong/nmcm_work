#include <iostream>

using namespace std;

class Dispatcher{

protected:
class SmsTracker{
private:
	SmsTracker(){cout<<"SmsTracker()"<<endl;}
	~SmsTracker(){cout<<"~SmsTracker()"<<endl;}
friend class Dispatcher;
};

public:
	Dispatcher(){cout<<"Dispatcher()"<<endl;}
	~Dispatcher(){cout<<"~Dispatcher()"<<endl;}


protected:
	SmsTracker* getSmsTracker(){ return new SmsTracker; }

public:
	void get()
	{ SmsTracker* p = getSmsTracker(); delete p; }
};

int main(int argc, const char* argv[])
{
	Dispatcher dispatcher;
	dispatcher.get();
	return 0;
}

