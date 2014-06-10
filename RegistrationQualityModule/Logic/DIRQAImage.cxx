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

DIRQAImage::DIRQAImage() {

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
	imageTypes["jacobian"]=JACOBIAN;/* <<-- WTF!!*/
}

void DIRQAImage::readFromXML(TiXmlNode const& n) {

	TiXmlElement const* nElement = n.ToElement();
	if(!nElement) {
		cout << "Not an element" << endl;
		throw std::runtime_error("Error: Cannot construct DIRQAImage, XMLNode is not of type element");
	} /*else cout << "DIRQAImage(TiXmlNode): Node is Element" << endl;*/

	switch(imageTypes[nElement->Value()]) {
		case IMAGE:
			cout << "DIRQAImage(TiXmlNode): Node is \"image\"" << endl;
			imageType = IMAGE;
			break;
		case WARPED:
			cout << "DIRQAImage(TiXmlNode): Node is \"warped\"" << endl;
			imageType = WARPED;
			break;
		case VECTOR:
			cout << "DIRQAImage(TiXmlNode): Node is \"vector\"" << endl;
			imageType = VECTOR;
			break;
		default:
			throw std::runtime_error("Error: Cannot construct DIRQAImage, element must be \"image\", \"warped\" or \"vector\"");
	}

	if(nElement->QueryIntAttribute( "index", &movingIndex ) != TIXML_SUCCESS) {
		throw std::runtime_error("Error: Integer attribute \"index\" is mandatory in element of type \"image\"");
	}

	if((imageType == WARPED || imageType == VECTOR)
		&& nElement->QueryIntAttribute( "reference", &fixedIndex ) != TIXML_SUCCESS) {
		throw std::runtime_error("Error: Integer attribute \"reference\" is mandatory in element of type \"image\"");
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

bool DIRQAImage::isLoaded() {
	return loaded;
}

void DIRQAImage::load(vtkSlicerVolumesLogic* volumesLogic) {
	if(!isLoaded()) {
		volumesLogic->AddArchetypeVolume(fileName.c_str(), tag.c_str(), 0, NULL);
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
			image.comment << " " << image.fileName;

	return os;
}
