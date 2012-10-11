#include "tinyxml2.h"
#include "xmltest.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined( _MSC_VER )
#include <direct.h>		// _mkdir
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
_CrtMemState startMemState;
_CrtMemState endMemState;
#else
#include <sys/stat.h>	// mkdir
#endif

using namespace tinyxml2;
int gPass = 0;
int gFail = 0;


// char_sep_example_3.cpp
// http://www.boost.org/doc/libs/1_40_0/libs/tokenizer/char_separator.htm

#include <iostream>
#include <boost/tokenizer.hpp>
#include <string>
#include <map>

int example_1( const char* filename )
{
	XMLDocument doc;
	doc.LoadFile( filename );
    
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" \t\n¡!¿?⸘‽“”‘’‛‟.,‚„'\"′″´˝^°¸˛¨`˙˚ªº…:;&_¯­–‑—§#⁊¶†‡@%‰‱¦|/\\ˉˆ˘ˇ-‒~*‼⁇⁈⁉$€¢£‹›«»<>{}[]()=+|");
    
    std::map<std::string, int> term_count_map;
    std::map<std::string, int>::iterator term_count_it;

    std::map<std::string, std::vector<int> > term_indexVec_map;
    std::map<std::string, std::vector<int> >::iterator term_indexVec_it;
    std::map<std::string, int> docID_index_map;
    
    int docIndex = 0;

    for (XMLElement* documentElement = doc.FirstChildElement("documents")->FirstChildElement("document"); 
         documentElement; 
         documentElement = documentElement->NextSiblingElement("document")) 
    {
        // Extract the document ID from the XML
        XMLElement* docID = documentElement->FirstChildElement("docID");
        std::string id_str(docID->GetText());
        // Set up hash map of docID string and index which will be used in count vectors
        docID_index_map[id_str] = docIndex;
        // Extract the document text from the XML
        XMLElement* docText = documentElement->FirstChildElement("docText");
        std::string text_str(docText->GetText());
        tokenizer tokens(text_str, sep);
        for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        {
            // std::cout << "<" << *tok_iter << "> ";
            std::string tmp = *tok_iter;
            if (tmp.length() > 2) {
                term_count_it = term_count_map.find(tmp);
                if (term_count_it == term_count_map.end()) {
                    // Initialize term count and doc index vector maps for new term
                    term_count_map[tmp] = 0;
                    std::vector<int> newvec;
                    term_indexVec_map[tmp] = newvec;
                }
                term_count_map[tmp]++;
                term_indexVec_map[tmp].push_back(docIndex);
            }
        }
        docIndex++;
        // std::cout << "\n";
    }
    for ( term_count_it=term_count_map.begin() ; term_count_it != term_count_map.end(); term_count_it++ )
    {
        std::cout << (*term_count_it).first << " => " << (*term_count_it).second << std::endl;
        std::vector<int> index_vec = term_indexVec_map[(*term_count_it).first];
        for ( int ii = 0; ii < index_vec.size(); ii++ )
        {
            std::cout << index_vec[ii] << " ";
        }
        std::cout << std::endl;
   }
    std::cout << term_count_map.size() << " terms in dictionary" << std::endl;

    
	return doc.ErrorID();
}

void example_5()
{
    printf( "XML Example 5\n" );
    
    // Test: Programmatic DOM
    // Build:
    //		<element>
    //			<!--comment-->
    //			<sub attrib="1" />
    //			<sub attrib="2" />
    //			<sub attrib="3" >& Text!</sub>
    //		<element>
    
    XMLDocument* doc = new XMLDocument();
    XMLNode* element = doc->InsertEndChild( doc->NewElement( "element" ) );
    
    XMLElement* sub[3] = { doc->NewElement( "sub" ), doc->NewElement( "sub" ), doc->NewElement( "sub" ) };
    for( int i=0; i<3; ++i ) {
        sub[i]->SetAttribute( "attrib", i );
    }
    element->InsertEndChild( sub[2] );
    XMLNode* comment = element->InsertFirstChild( doc->NewComment( "comment" ) );
    element->InsertAfterChild( comment, sub[0] );
    element->InsertAfterChild( sub[0], sub[1] );
    sub[2]->InsertFirstChild( doc->NewText( "& Text!" ));
    doc->Print();
    
    doc->SaveFile( "pretty.xml" );
    doc->SaveFile( "compact.xml", true );
    delete doc;
}


int main( int argc, const char** argv )
{
#if defined( _MSC_VER ) && defined( DEBUG )
    _CrtMemCheckpoint( &startMemState );
#endif
    
//    if (argc != 2) {
//        printf("must specify filename\n");
//        return EXIT_FAILURE;
//    }
    const char* filename = "/Users/emonson/Downloads/Jigsaw/datafiles/VAST2007Contest.jig";
	example_1( filename );
//    example_5();


    return EXIT_SUCCESS;
}
