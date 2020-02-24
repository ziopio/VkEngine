#pragma once

class Project
{
public:
	Project(const char* project_dir);
	void load();
	void save();
	~Project();
private:
	const char* project_dir;
	struct _data;
	_data* data;
};