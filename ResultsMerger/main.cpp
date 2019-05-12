#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>

struct Item
{
	char name[128];
	int raw, gaze;
	int linear, fov;
	int radius[3];
	int quality[3];
};

int main(int argc, char** argv)
{
	assert(argc == 2 && "New two arguments");
	char buff[256];
	const char* file = argv[1];

	// Parse the new items
	std::ifstream newData;
	newData.open(file);

	assert(newData.good() == true && "Failed to open newdata");
	newData.getline(buff, 256);
	newData.getline(buff, 256);
	int counter = 0;

	Item newItems[10];
	while(counter < 10 && newData.good())
	{
		Item& item = newItems[counter++];

		newData >> item.name >> item.raw >> item.gaze >> item.linear >> item.fov >>
			item.radius[0] >> item.radius[1] >> item.radius[2] >> item.quality[0] >>
			item.quality[1] >> item.quality[2];
	}

	newData.close();

	// Get the old items
	Item oldItems[10];
	newData.open("TotalResults.txt");

	counter = 0;
	if(false == newData.good())
	{
		return 1;
	}
	newData.getline(buff, 256);
	while(counter < 10 && newData.good())
	{
		Item& item = oldItems[counter++];

		newData >> item.name >> item.raw >> item.gaze >> item.linear >> item.fov >>
			item.radius[0] >> item.radius[1] >> item.radius[2] >> item.quality[0] >>
			item.quality[1] >> item.quality[2];
	}
	newData.close();
	// increment the old values with the new
	for(int i = 0; i < 10; i++)
	{

		Item& oldItem = oldItems[i];
		// Find the item with the same name
		for(int j = 0; j < 10; j++)
		{
			Item& newItem = newItems[j];
			if(strcmp(oldItem.name, newItem.name) == 0)
			{
				oldItem.raw += newItem.raw;
				oldItem.gaze += newItem.gaze;

				oldItem.linear += newItem.linear;
				oldItem.fov += newItem.fov;
				for(int k = 0; k < 3; k++)
				{
					oldItem.radius[k] += newItem.radius[k];
					oldItem.quality[k] += newItem.quality[k];
				}
				break;
			}
		}
	}

	std::ofstream outputData;
	outputData.open("TotalResults.txt");
	outputData
		<< "Image\tRAW\tGAZE\tLinear\tFOV\tRadius 1\tRadius 0.8\tRadius 0.6\tQuality 1\tQuality "
		   "0.8\tQuality 0.6\n";
	for(int i = 0; i < 10; i++)
	{
		outputData << oldItems[i].name << "\t" << oldItems[i].raw << "\t" << oldItems[i].gaze
				   << "\t"

				   << oldItems[i].linear << "\t" << oldItems[i].fov << "\t" << oldItems[i].radius[0]
				   << "\t" << oldItems[i].radius[1] << "\t" << oldItems[i].radius[2] << "\t"
				   << oldItems[i].quality[0] << "\t" << oldItems[i].quality[1] << "\t"
				   << oldItems[i].quality[2] << "\n";
	}
	outputData.close();


}
