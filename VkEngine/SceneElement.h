#pragma once
#include <string>

namespace vkengine
{

	enum SceneElementType{

	};

	class SceneElement
	{
	public:
		SceneElement(unsigned id, std::string name)
		{
			this->id = id; this->name = name;
		};
		inline unsigned getId() { return id; };
		//inline std::string & getName() { return name; };
		//inline void setName() { this->name = name; };
		std::string name;
	protected:
		unsigned id;

	};
}