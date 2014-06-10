#pragma once

#include <string>
#include <map>
#include <tinyxml.h>

#include <vtkObject.h>

class vtkSlicerVolumesLogic;

class DIRQAImage : public vtkObject {

public:

	enum DIRQAImageType {
		IMAGE = 0, WARPED, VECTOR, JACOBIAN,
	};

	static std::map<std::string,DIRQAImageType> imageTypes;

	static DIRQAImage* New();

	static void initHashMap();

	DIRQAImage();
	DIRQAImage(TiXmlNode const& n);
	DIRQAImage(DIRQAImage::DIRQAImageType imageType, std::string const& fileName, std::string const& tag, std::string const& comment, int movingIndex, int fixedIndex=-1);

	~DIRQAImage();

	void readFromXML(TiXmlNode const& n);
 	bool isLoaded();
	void load(vtkSlicerVolumesLogic*);

	friend std::ostream& operator<<(std::ostream& os, const DIRQAImage& image);

private:
	DIRQAImageType imageType;
	std::string fileName;
	std::string tag;
	std::string comment;
	int movingIndex; // Also image index
	int fixedIndex;

	std::string volumeNodeID;
	bool loaded;
};