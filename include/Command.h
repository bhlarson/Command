#pragma once
#include <future>
#include <functional>
#include <memory>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>

struct CommandData
{
	virtual ~CommandData() {};
};

class Command
{
public:
	Command() {};
	virtual ~Command() {};
	virtual void Start(std::shared_ptr<CommandData> init, std::function<void(std::shared_ptr<CommandData>)> Complete)
	{
		std::shared_ptr<CommandData> result = std::make_shared<CommandData>();
		Complete(result);
	}
	virtual void Stop(std::shared_ptr<CommandData> stopData)=0;

	std::shared_ptr<CommandData> State() {
		return m_state;
	}
protected:
	std::shared_ptr<CommandData> m_state;
};

