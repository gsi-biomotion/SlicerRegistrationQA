project(tinyXML)

#add_definitions(-DTIXML_USE_STL)
add_library(tinyXML tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp)