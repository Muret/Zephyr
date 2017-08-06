#include "Utilities.h"
#include "Camera.h"
#include "KeyChain.h"

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

string Utilities::get_extension_from_path(string path)
{
	string ext = path.substr(path.find_last_of(".") + 1, path.length());
	return ext;
}

D3DXVECTOR4 Utilities::get_debug_vector()
{
	return debug_vector;
}

float Utilities::random_normalized_float(int presicion /*= RAND_MAX*/)
{
	return float(rand() % presicion) / (float)RAND_MAX;
}

void Utilities::tick()
{
	float multiplier = 3;

	if (key_chain.key('1'))
	{
		debug_vector.x += 0.01 * multiplier;
	}
	if (key_chain.key('2'))
	{
		debug_vector.y += 0.01 * multiplier;
	}
	if (key_chain.key('3'))
	{
		debug_vector.z += 0.01 * multiplier;
	}
	if (key_chain.key('4'))
	{
		debug_vector.w += 0.01 * multiplier;
	}

	if (key_chain.key('5'))
	{
		debug_vector.x -= 0.01 * multiplier;
	}
	if (key_chain.key('6'))
	{
		debug_vector.y -= 0.01 * multiplier;
	}
	if (key_chain.key('7'))
	{
		debug_vector.z -= 0.01 * multiplier;
	}
	if (key_chain.key('8'))
	{
		debug_vector.w -= 0.01 * multiplier;
	}
}


D3DXVECTOR3 operator*(const D3DXVECTOR3 &lhs, const D3DXVECTOR3 &rhs)
{
	D3DXVECTOR3 output;
	output.x = lhs.x * rhs.x;
	output.y = lhs.y * rhs.y;
	output.z = lhs.z * rhs.z;

	return output;
}

float vec3_len(const D3DXVECTOR3 &vec)
{
	return D3DXVec3Length(&vec);
}

ScopeTimer::ScopeTimer(string name)
{
	start = std::chrono::system_clock::now();
	name_ = name;
}

ScopeTimer::~ScopeTimer()
{
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	double ms = elapsed_seconds.count() * 1000;
}
