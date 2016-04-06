#include "DIRQAImage.h"
#include <iostream>
#include <stdexcept>

#include "vtkSlicerVolumesLogic.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

DIRQAImage::DIRQAImage() {

}

DIRQAImage::DIRQAImage(TiXmlNode const& n)
: volumeNodeID("")
, loaded(false) {

	TiXmlElement const* nElement = n.ToElement();
	if(!nElement) {
		cout << "Not an element" << endl;
		throw std::runtime_error("Error: Cannot construct DIRQAImage, XMLNode is not of type element");
	} else cout << "DIRQAImage(TiXmlNode): Node is Element" << endl;

	if(nElement->Value()==string("image")) {
		cout << "DIRQAImage(TiXmlNode): Node is \"image\"" << endl;
		imageType = IMAGE;
	} else {
		throw std::runtime_error("Error: Cannot construct DIRQAImage, element must be \"image\"");
	}

	if(nElement->QueryIntAttribute( "index", &movingIndex ) != TIXML_SUCCESS) {
		throw std::runtime_error("Error: Integer attribute \"index\" is mandatory in element of type \"image\"");
	}

	const TiXmlElement* tmp = nElement->FirstChildElement("file");
	if(!tmp) {
		throw std::runtime_error("Error: Element of type \"image\" needs child \"file\"");
	}
	fileName = tmp->GetText();

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
