#include <iostream>
#include <thread>
#include "Editor.h"

int main(int argc, char* argv) {

	std::thread editor_thread(
	[]
	{ 
		Editor e; 
		e.execute(); 
	});

	editor_thread.join();

	std::cout << "Press enter to return from main()..." << std::endl;
	std::getchar();

	return 0;
}