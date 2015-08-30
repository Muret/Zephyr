
#ifndef USE_TextureLoader_H_
#define USE_TextureLoader_H_

#include <string>

class Texture;

class TextureLoader
{
public:

	TextureLoader(std::string filename);

	Texture* create_texture_from_file();

private:

	std::string filename_;

};

#endif