#ifndef INCLUDE_UTILITIES_
#define INCLUDE_UTILITIES_

#include "includes.h"

class Utilities
{
public:

	Utilities();
	~Utilities();

	static void get_files_under_folder(string path, vector<string> &names, const vector<string> &allowed_extensions);
	static void get_folders_under_folder(string path, vector<string> &names);
	static string get_file_name_from_path(string path);
	static string get_file_name_from_path_wo_extension(string path);

};

#endif