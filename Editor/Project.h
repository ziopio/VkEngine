#pragma once
#include <memory>
#include <string>

class Project
{
public:
	Project(const char* project_dir);
	void load();
	void save();
	~Project();
private:
	struct _data;
	std::unique_ptr<_data> data;
};