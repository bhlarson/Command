// ActionTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Command.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include <memory>

using namespace std;

struct SleepInitData : public CommandData
{
public:
	SleepInitData(unsigned int initSleepMs=2000):sleepMs(initSleepMs){}
	virtual ~SleepInitData() {};
	unsigned int SleepMs() { return sleepMs; };
	void SleepMs(int ms) { sleepMs = ms; }
	unsigned int sleepMs;
	template <class Archive> void serialize(Archive & ar)
	{
		ar(cereal::base_class<CommandData>(this), sleepMs);
	}
};

struct SleepState : public CommandData
{
public:
	SleepState() :sleepMs(0) {}
	unsigned int sleepMs;
	void Initialize(SleepInitData& init)
	{
		sleepMs = init.sleepMs;
	}
	
	template <class Archive> void serialize(Archive & ar)
	{
		ar(cereal::base_class<Base>(CommandData), sleepMs);
	}
	//template <class Archive> void save(Archive & ar)
	//{
	//	ar(sleepMs);
	//}
	//template <class Archive> void load(Archive & ar)
	//{
	//	ar(sleepMs);
	//}
};

struct BaseClass
{
	virtual void sayType() = 0;
};

// A class derived from BaseClass
struct DerivedClassOne : public BaseClass
{
	void sayType()
	{
		std::cout << "DerivedClassOne" << std::endl;
	}

	int x;

	template<class Archive> void serialize(Archive & ar)
	{
		ar(cereal::base_class<BaseClass>(this), x);
	}
};

CEREAL_REGISTER_TYPE(SleepInitData);
CEREAL_REGISTER_TYPE_WITH_NAME(SleepState,"SleepState");
CEREAL_REGISTER_TYPE(DerivedClassOne);

class SleepCommand : public Command
{
public:
	SleepCommand() 
	{
		m_sleepState = std::make_shared<SleepState>();
		m_state = m_sleepState;
	};
	virtual ~SleepCommand() {
		if (thread_ptr)
		{
			thread_ptr->join();
			cout << "~SleepCommand join complete" << std::endl;
		}
	};

	virtual void Start(std::shared_ptr<CommandData> init, std::function<void(std::shared_ptr<CommandData>)> Complete)
	{
		auto sleepInit = dynamic_pointer_cast<SleepInitData>(init);
		m_sleepState->sleepMs = sleepInit->SleepMs();
		m_Complete = Complete;
		thread_ptr = std::unique_ptr<std::thread>(new std::thread([this]() {
			cout << "In SleepThread Lamda" << std::endl;
			std::chrono::milliseconds duration(m_sleepState->sleepMs);
			std::this_thread::sleep_for(duration);
			auto result = std::make_shared<CommandData>();
			this->m_Complete(result);
		}));
    // spawn new thread that calls foo()

	}
	void Stop(std::shared_ptr<CommandData>) {};
protected:
	std::function<void(std::shared_ptr<CommandData>)> m_Complete;
	std::unique_ptr<std::thread> thread_ptr;
	std::shared_ptr<SleepState> m_sleepState;
};

int main()
{
	// Composition
	std::shared_ptr<CommandData> initData = std::make_shared<SleepInitData>();
	SleepCommand  sleepCommand;
	sleepCommand.Start(initData, [](std::shared_ptr<CommandData> result)
	{
		SleepCommand  nextCommand;
		std::shared_ptr<CommandData> initData1 = std::make_shared<SleepInitData>();
		cout << "In Start Lambda" << std::endl;
		nextCommand.Start(initData1, [](std::shared_ptr<CommandData> nextResult) {
			cout << "In Next Lambda" << std::endl;
		});
	});

	DerivedClassOne* derivedClass = new DerivedClassOne();
	std::ofstream os("cereal.json", std::ofstream::out);
	cereal::JSONOutputArchive archive(os);
	archive(*derivedClass);

	delete derivedClass;

    return 0;
}

