#include <iostream>

#include "TexData.hpp"
using namespace std;

int main()
{
	try
	{
		auto config = TexData::LoadFile("test.txt");
		config.DebugPrint();
		cout << "\n\n" << config["Scene"]["Files"]["SaveTo"].as<string>() << endl;
		for(auto& c : config["Scene"]["Dependencys"])
		{
			cout << c->as<string>() << endl;
		}

		auto Scene = config["Scene"];
		cout << "Overwrite: " << Scene["Files"].has("overwrite") << endl;

	}
	catch(std::exception& ex)
	{
		cout << ex.what() << endl;
	}
	system("pause");
}