#include "Utilities.h"
#include "Camera.h"

D3DXVECTOR4 Utilities::debug_vector = D3DXVECTOR4(0, 0, 0, 0);

Utilities::Utilities()
{
}

Utilities::~Utilities()
{

}

void Utilities::get_files_under_folder(string path, vector<string> &names, const vector<string> &allowed_extensions)
{
	std::wstring stemp = std::wstring(path.begin(), path.end());
	LPCWSTR search_path = stemp.c_str();

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFileA((path + "/*").c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		while (1)
		{
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{
				string file_name(fd.cFileName);

				bool valid_file = true;

				if (allowed_extensions.size() > 0)
				{
					string ext = file_name.substr(file_name.find_last_of(".") + 1, file_name.length());
					if (find(allowed_extensions.begin(), allowed_extensions.end(), ext) == allowed_extensions.end())
					{
						valid_file = false;
					}
				}

				if (valid_file)
				{
					names.push_back(file_name);
				}
			}
			if (FindNextFile(hFind, &fd) == false)
			{
				break;
			}
		} 
		FindClose(hFind);
	}
}

void Utilities::get_folders_under_folder(string path, vector<string> &names)
{
	std::wstring stemp = std::wstring(path.begin(), path.end());
	LPCWSTR search_path = stemp.c_str();

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFileA((path + "/*").c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (1)
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				string file_name(fd.cFileName);
				if (file_name != "." && file_name != "..")
				{
					names.push_back(file_name);
				}
			}
			if (FindNextFile(hFind, &fd) == false)
			{
				break;
			}
		}
		FindClose(hFind);
	}
}

std::string Utilities::get_file_name_from_path(string path)
{
	int index = path.find_last_of('/');
	if (index == string::npos)
	{
		index = path.find_last_of('\\');
	}

	if (index == string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(index + 1, path.length());
	}

}

std::string Utilities::get_file_name_from_path_wo_extension(string path)
{
	int index = path.find_last_of('/');
	if (index == string::npos)
	{
		index = path.find_last_of('\\');
	}

	string filename = path;
	if (index != string::npos)
	{
		filename = path.substr(index + 1, path.length());
	}

	index = filename.find_last_of('.');
	if (index != string::npos)
	{
		filename = filename.substr(0, index);
	}

	return filename;
}

D3DXVECTOR4 Utilities::get_debug_vector()
{
	return debug_vector;
}

void Utilities::tick()
{
	extern Camera demo_camera;
	if (demo_camera.is_key_down('1'))
	{
		debug_vector.x += 0.01;
	}
	if (demo_camera.is_key_down('2'))
	{
		debug_vector.y += 0.01;
	}
	if (demo_camera.is_key_down('3'))
	{
		debug_vector.z += 0.01;
	}
	if (demo_camera.is_key_down('4'))
	{
		debug_vector.w += 0.01;
	}

	if (demo_camera.is_key_down('5'))
	{
		debug_vector.x -= 0.01;
	}
	if (demo_camera.is_key_down('6'))
	{
		debug_vector.y -= 0.01;
	}
	if (demo_camera.is_key_down('7'))
	{
		debug_vector.z -= 0.01;
	}
	if (demo_camera.is_key_down('8'))
	{
		debug_vector.w -= 0.01;
	}
}
