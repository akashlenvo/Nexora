#pragma once

#include <string>
#include <map>
#include <vector>

namespace Video
{
	struct Filter
	{
		enum Category
		{
			NONE, 
			CORRECTION, 
			EFFECT, 
			DISTORTION, 
			ARTISTIC
		};

		using Registry = std::map<Category, std::vector<std::string>>;
	};
}