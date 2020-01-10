#pragma once

#include <string>

bool ConvertPdfToJpeg(
	std::string InputPath,
	std::string OutputPath,
	int StartPage,
	int EndPage,
	int Dip
);
