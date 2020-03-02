#pragma once
#include <string>

namespace vkengine
{

	enum SceneElementType{

	};

	class SceneElement
	{
	public:
		SceneElement(std::string id, std::string name)
		{
			this->id = id; this->name = name;
		};
		inline std::string getId() { return id; };
		std::string name;
	protected:
		std::string id;
	};
}