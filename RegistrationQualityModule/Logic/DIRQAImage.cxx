#include "DIRQAImage.h"
#include <iostream>
#include <stdexcept>

#include "vtkObjectFactory.h"
#include "vtkSlicerVolumesLogic.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

std::map<std::string,DIRQAImage::DIRQAImageType> DIRQAImage::imageTypes;

DIRQAImage::DIRQAImage()
: volumeNodeID("")
, loaded(false){

}

DIRQAImage::DIRQAImage(TiXmlNode const& n)
: volumeNodeID("")
, loaded(false) {
	readFromXML(n);
}

vtkStandardNewMacro(DIRQAImage);

void DIRQAImage::initHashMap() {
	imageTypes["image"]=IMAGE;
	imageTypes["warped"]=WARPED;
	imageTypes["vector"]=VECTOR;
	imageTypes["jacobian"]=JACOBIAN;
}

bool DIRQAImage::compareOnIndex(vtkSmartPointer<DIRQAImage> lhs, vtkSmartPointer<DIRQAImage> rhs) {
	return lhs->movingIndex < rhs->movingIndex;
}

bool DIRQAImage::compareOnFixedIndex(vtkSmartPointer<DIRQAImage> lhs, vtkSmartPointer<DIRQAImage> rhs) {
	return lhs->fixedIndex < rhs->fixedIndex;
}

void DIRQAImage::readFromXML(TiXmlNode const& n) {

	TiXmlElement const* nElement = n.ToElement();
	if(!nElement) {
		cout << "Not an element" << endl;
		throw std::runtime_error("Error: Cannot construct DIRQAImage, XMLNode is not of type element");
	} /*else cout << "DIRQAImage(TiXmlNode): Node is Element" << endl;*/

	try {
		imageType = imageTypes.at(nElement->Value());
	} catch (std::out_of_range ex) {
		throw std::runtime_error("Error: Invalid XML-tag");
	}

	//TODO: jacobian, ... and throw this out
	if( imageType!=IMAGE && imageType!=VECTOR && imageType!=WARPED) {
		throw std::runtime_error("Error: Invalid element in xml-File");
	}

	if(nElement->QueryIntAttribute( "index", &movingIndex ) != TIXML_SUCCESS) {
		throw std::runtime_error("Error: Integer attribute \"index\" is mandatory in element of type \"image\"");
	}

	//TODO add index sanity check (negative values, ...)

	if(imageType == WARPED || imageType == VECTOR) {
		if(nElement->QueryIntAttribute( "reference", &fixedIndex ) != TIXML_SUCCESS) {
			throw std::runtime_error("Error: Integer attribute \"reference\" is mandatory in element of type \"image\"");
		}
	} else fixedIndex = 0;

	const TiXmlElement* tmp = nElement->FirstChildElement("file");
	if(!tmp) {
		throw std::runtime_error("Error: Element of type \"image\" needs child \"file\"");
	}
	fileName = tmp->GetText();

	tmp = nElement->FirstChildElement("tag");
	if(!tmp) {
		throw std::runtime_error("Error: Element of type \"image\" needs child \"tag\"");
	}
	tag = tmp->GetText();

	tmp = nElement->FirstChildElement("comment");
	if(!tmp)	comment = "";
	else		comment = tmp->GetText();

}

DIRQAImage::DIRQAImage(DIRQAImageType type, std::string const& filename,
					   std::string const& Tag, std::string const& Comment,
					   int movIndex, int fixIndex)
: imageType(type)
, fileName(filename)
, tag(Tag)
, comment(Comment)
, movingIndex(movIndex)
, fixedIndex(fixIndex)
, volumeNodeID("")
, loaded(false) {

}

// void dump(TiXmlNode* n) {
// 	int type = n->Type();
// 	cout << "Type=" << type << endl;
// 	cout << n->Value() << endl;
// 	if(!(type==TINYXML_ELEMENT)) {
// 		cout << "Not an element" << endl;
// 	}
// 	for(TiXmlNode *child=n->FirstChild(); child!=0;child=child->NextSibling()) {
// 		dump(child);
// 	}
// }

DIRQAImage::~DIRQAImage() {

}

DIRQAImage::DIRQAImageType DIRQAImage::getImageType() {
	return imageType;
}

std::string DIRQAImage::getNodeID() {
	return volumeNodeID;
}

std::string DIRQAImage::getTag() {
	return tag;
}

int DIRQAImage::getIndex() {
	return movingIndex;
}

int DIRQAImage::getFixedIndex() {
	return fixedIndex;
}

std::string const& DIRQAImage::getFileName() const {
	return fileName;
}

bool DIRQAImage::isLoaded() {
	return loaded;
}

void DIRQAImage::load(vtkSlicerVolumesLogic* volumesLogic) {
	cout << "Loading" << endl;
	if(!isLoaded()) {
		cout << "Really Loading" << endl;
		volumeNodeID = volumesLogic->AddArchetypeVolume(fileName.c_str(), tag.c_str(), 0, NULL)->GetID();
		cout << "Done Loading" << endl;
	}
}

std::ostream& operator<<(std::ostream& os, const DIRQAImage& image) {
	switch(image.imageType) {
		case DIRQAImage::IMAGE:		os << "IMAGE"; break;
		case DIRQAImage::WARPED: 	os << "WARPED"; break;
		case DIRQAImage::VECTOR:	os << "VECTOR"; break;
		case DIRQAImage::JACOBIAN:	os << "JACOBIAN"; break;
		default: std::cerr << "Error: Unknown image Type!" << std::endl; return os;
	}
	os << 	 " " << image.movingIndex << " " <<
			image.fixedIndex << " " << image.tag << " " <<
			image.comment << " " << image.fileName << " " << image.volumeNodeID;

	return os;
}
