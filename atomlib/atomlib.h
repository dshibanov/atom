#ifndef ATOMLIB_H
#define ATOMLIB_H

#include "basics.h"
#include "glyph.h"
#include "options.h"
#include "font.h"
#include "curve.h"
#include "blend.h"
#include "parser.h"
#include <sstream>
#include <map>

using namespace fg;
#include "composedglyph.h"
#include "aglyph.h"

typedef std::map<Atom*, Ints> Stats;

#define MIN_ATOMS 0
#define MIN_USES_IN_GLYPHS 1
#define MIN_USES_IN_DIFFERENT_GLYPHS 2

class AtomLib
{
	
public:
    static const int SCALE_XY[4][2];


    bool 	compareBBox; // compare or not
    bool    allGlyphs; // process all glyphs
    int 	from;
    int 	to;

    int 	splineLength;
    double 	tolerance;
    double 	toleranceBBox;
    double 	toleranceEndPoints;
    int		minAtomsCount; // min number raw atoms in atom
    int		minUsedCount; // min number of glyphs in which atom has to be used

    int decompose(fg::Font* font, XGlyphs &aglyphs, Atoms &cdict);//, Atoms &sourceAtomsDict, AGlyphs &agGlyphs);
    int compressXGlyphs(XGlyphs &aglyphs, Atoms &cdict);
    int compress(fg::Font* font, XGlyphs &aglyphs, Atoms &cdict);
    int stats(fg::Font* font, XGlyphs &aglyphs, Atoms &regularDict, Atoms &statDict);
    int stats2(fg::Font* font, XGlyphs &aglyphs, Atoms &regularDict, list<Atom*> &statLinks);

    int removeExcessAtoms(Atoms &dict, int maCount);
    int removeExcessAtoms2(Atoms &dict, int value, int type = MIN_ATOMS);

    Ints getUsedIn(XGlyphs &agdict, Atom &a);
    Point getFirstJoiningPointOfXNode(XNode &xn);




    //------
    int substituteSimpleNodes(XContour &xc);

	
	AtomLib();
	



	
private:


    Atoms *atomsDict;// dict of atoms
    XGlyphs *agGlyphs;// dict of aglyphs
    list<Representations> representationsDict;

    list<Atom*> suitableAtomsIndexes(Atoms &dict, int maCount);

    int getRepresentationsFromContour(const Contour &c, Representations &rs, int len, bool reverse);
    Splines contourToSplines(const Contour &contour, int len, Matrix m = Matrix(1,0,0,1), bool reverse = false);
    int getDictIndexOfAtom(const Contour &atom, Atoms &atomsDict, Matrix &m, int &reverse);    
    int findComposition(XContour &source, XNodes::iterator &sourceStart, XContour &analyzed, XNodes::iterator &analyzedStart, Atoms &atomsDict, bool _addedToDict = false);
    int deleteUnUsed(XGlyphs &agdict, Atoms &dict);
    int getSequenceFinishNode(const XContour &contour, XNodes::iterator &finishNode);    
    int addToDict(Atoms &dict, Atom &a);
    bool isSameSequences(Ints seq1, Ints seq2, bool &reverse);
    int fillInsertPoints(XGlyphs &glyphs, list<Atom*> &dict);
    XGlyph* findGlyphByIndex(XGlyphs &glyphs, int index);
    InsertPoints findIn(XGlyph &g, Atom *a);
    void printips(InsertPoints &ips);    
};





#endif // ATOMLIB_H
