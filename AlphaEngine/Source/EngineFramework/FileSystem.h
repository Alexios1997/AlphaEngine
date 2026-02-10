#pragma once

#include <filesystem>

namespace AlphaEngine
{
	class FileSystem 
	{
	public:
		static std::string GetPath(const std::string& relativePath)
		{
			std::filesystem::path root(ALPHA_SOURCE_DIR);
			return (root / relativePath).string();
		}
	};
}