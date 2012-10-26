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
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <string>
#include <map>

#include "ANN.h"

int example_1( const char* filename )
{
	XMLDocument doc;
	doc.LoadFile( filename );
    
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" \t\n¡!¿?⸘‽“”‘’‛‟.,‚„'\"′″´˝^°¸˛¨`˙˚ªº…:;&_¯­–‑—§#⁊¶†‡@%‰‱¦|/\\ˉˆ˘ˇ-‒~*‼⁇⁈⁉$€¢£‹›«»<>{}[]()=+|");
    
    // read in stopwords from text file
    std::ifstream stopfile("english_stopwords", std::ios_base::in);
    assert(!stopfile.fail());
    std::map<std::string, bool> stopwords_map;
    
    // load stopwords into hash map
    std::string s;
    stopfile >> s;
    while (!stopfile.eof()) {
        stopwords_map[s] = true;
        stopfile >> s;
    }
    stopfile.close();
    // assert(!stopfile.fail());
    
    std::map<std::string, int> term_count_map;
    std::map<std::string, int>::iterator term_count_it;

    std::map<std::string, std::vector<int> > term_indexVec_map;
    std::map<std::string, std::vector<int> >::iterator term_indexVec_it;
    
    std::map<int, std::string> index_docID_map;
    std::map<int, std::string> index_term_map;
    
    // CONSTANTS
    int MIN_TERM_LENGTH = 2;
    int MIN_TERM_COUNT = 2;
    
    int docIndex = 0;

    for (XMLElement* documentElement = doc.FirstChildElement("documents")->FirstChildElement("document"); 
         documentElement; 
         documentElement = documentElement->NextSiblingElement("document")) 
    {
        // Extract the document ID from the XML
        XMLElement* docID = documentElement->FirstChildElement("docID");
        std::string id_str(docID->GetText());
        
        // Set up hash map of docID string and index keys which will be used in count vectors
        index_docID_map[docIndex] = id_str;
        
        // Extract the document text from the XML
        XMLElement* docText = documentElement->FirstChildElement("docText");
        std::string text_str(docText->GetText());
        tokenizer tokens(text_str, sep);
        for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        {
            // std::cout << "<" << *tok_iter << "> ";
            std::string tmp = *tok_iter;
            // NOTE: Right now doing a rough length check
            if (tmp.length() > MIN_TERM_LENGTH) {
                // Only count terms not in stopwords list
                if (!stopwords_map.count(tmp)) {
                    // Check for all caps, otherwise convert to lowercase
                    //   (maybe should just be turning everything to lowercase...)
                    if (!boost::all(tmp, boost::is_upper())) {
                        boost::to_lower(tmp);
                    }
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
        }
        docIndex++;
        // std::cout << "\n";
    }
    
    // Now that we have the terms and the documents they came from, we need to 
    // create the actual term-document vectors out of ANN data structures
    
    int nPts = 0;                               // actual number of data points
    int dim = 0;                                // dimensionality
	ANNpointArray		dataPts;				// data points
	ANNpoint			queryPt;				// query point
	ANNidxArray			nnIdx;					// near neighbor indices
	ANNdistArray		dists;					// near neighbor distances
	ANNkd_tree*			kdTree;					// search structure
        
	nPts = docIndex;    // TODO: check for off by one errors!!
    dim = int(term_count_map.size());

	queryPt = annAllocPt(dim);					// allocate query point
    
    // Allocate the space for points and create array of pointers to coordinate arrays (each dim long)
	dataPts = annAllocPts(nPts, dim);			// allocate data points
    
    // Initialize to zero (counts) since ANN code doesn't do this
    std::fill_n(dataPts[0], nPts*dim, 0);
    
    int term_idx = 0;
    
    // Run through all of the entries in the term totals and correpsonding doc index vectors
    for ( term_count_it=term_count_map.begin() ; term_count_it != term_count_map.end(); term_count_it++ )
    {
        // First, check if count passes threshold (could base this on percentiles in future...)
        if ((*term_count_it).second < MIN_TERM_COUNT)
        {
            continue;
        }
        
        // NOTE: Could set up here some sort of entropy thresholds
        
        // Record the term with its index as key
        index_term_map[term_idx] = (*term_count_it).first;
        
        // Print
        std::cout << (*term_count_it).first << " => " << (*term_count_it).second << std::endl;
        
        // Convenience vector so iteration and access are more clear
        std::vector<int> index_vec = term_indexVec_map[(*term_count_it).first];
        
        // Run through doc index vectors and increment counts in real data arrays
        for ( int ii = 0; ii < index_vec.size(); ii++ )
        {
            std::cout << index_vec[ii] << " ";
            // Increment count sums
            dataPts[index_vec[ii]][term_idx] += 1;
        }
        std::cout << std::endl;
        term_idx++;
    }
    std::cout << std::endl << term_count_map.size() << " terms in dictionary, " << term_idx << " terms used" << std::endl << std::endl;
    
    // DEBUG: Check counts in dataPts arrays
//    for ( int docIdx = 0; docIdx < nPts; docIdx++ )
//    {
//        for (int termIdx = 0; termIdx < dim; termIdx++)
//        {
//            std::cout << dataPts[docIdx][termIdx] << '.';
//        }
//        std::cout << std::endl;
//    }
    
    // Build the ANN kd-tree data structure for fast NN lookups
	kdTree = new ANNkd_tree(					// build search structure
                            dataPts,					// the data points
                            nPts,						// number of points
                            dim);						// dimension of space
    
//    kdTree->annkSearch(						// search
//                       queryPt,						// query point
//                       k,								// number of near neighbors
//                       nnIdx,							// nearest neighbors (returned)
//                       dists,							// distance (returned)
//                       eps);							// error bound

//	int annkFRSearch(					// approx fixed-radius kNN search
//                     ANNpoint		q,				// the query point
//                     ANNdist			sqRad,			// squared radius of query ball
//                     int				k,				// number of neighbors to return
//                     ANNidxArray		nn_idx = NULL,	// nearest neighbor array (modified)
//                     ANNdistArray	dd = NULL,		// dist to near neighbors (modified)
//                     double			eps=0.0);		// error bound
    
        
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
    const char* filename = "/Users/emonson/Downloads/Jigsaw/datafiles/InfoVisVASTPapers-95-09.jig";
	example_1( filename );
//    example_5();


    return EXIT_SUCCESS;
}
