#ifndef INCLUDE_UTILITIES_
#define INCLUDE_UTILITIES_

#include <Windows.h>
#include <iostream>
#include <sstream>
#include "includes.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#define SCOPE_TIMER(name) ScopeTimer timer(name)

class ScopeTimer
{
public:
	ScopeTimer(string name);
	~ScopeTimer();

	std::chrono::time_point<std::chrono::system_clock> start;


	string name_;
};

class Utilities
{
public:

	Utilities();
	~Utilities();

	static void get_files_under_folder(string path, vector<string> &names, const vector<string> &allowed_extensions);
	static void get_folders_under_folder(string path, vector<string> &names);
	static string get_file_name_from_path(string path);
	static string get_file_name_from_path_wo_extension(string path);
	static string get_extension_from_path(string path);

	static D3DXVECTOR4 get_debug_vector();

	static float random_normalized_float(int presicion = RAND_MAX);

	static void tick();

	static string formatted_string(const char* fmt, ...)
	{
		int size = 2048;
		char* buffer = 0;
		buffer = new char[size];
		va_list vl;
		va_start(vl, fmt);
		int nsize = vsnprintf(buffer, size, fmt, vl);
		if (size <= nsize) { //fail delete buffer and try again
			delete[] buffer;
			buffer = 0;
			buffer = new char[nsize + 1]; //+1 for /0
			nsize = vsnprintf(buffer, size, fmt, vl);
		}
		std::string ret(buffer);
		va_end(vl);
		delete[] buffer;
		return ret;
	}

private:
	static D3DXVECTOR4 debug_vector;

};

float vec3_len(const D3DXVECTOR3 &vec);

#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

#endif