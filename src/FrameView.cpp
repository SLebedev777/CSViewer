#include "FrameView.h"
#include <iostream>


void describe_frame(CSVContainer::Frame& frame)
{
	std::cout << std::endl;
	std::cout << "Shape: " << frame.getRowRanges().totalElements() << " rows x " <<
		frame.getColRanges().totalElements() << " cols" << std::endl;
}

void print_frame(CSVContainer::Frame& frame)
{
	std::cout << std::endl;
	std::cout << frame.getColumnNames() << std::endl;
	std::cout << "--------------------" << std::endl;
	for (const auto& row : frame)
	{
		std::cout << row << std::endl;
	}

	describe_frame(frame);
}
