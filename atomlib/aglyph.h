#ifndef ATOM_H
#define ATOM_H

#include <QDebug>

typedef vector<int> Ints;
typedef vector<double> Doubles;


struct InsertPoint
{
    string glyphName;
    int contourNum;
    int startNodeNum;
    int endNodeNum;
};

typedef list<InsertPoint> InsertPoints;


class Atom : public Contour
{
	public:

    InsertPoints insertPoints;
    int rawAtomsCount;
	Ints usedIn;	// in which glyphs this atom uses
	Ints components; // source atoms from which current atom constructed
    int contourIndex; // its own index assigned on creation
	
	inline int removeFromUsedIn(int glyph)
	{
		Ints::iterator it = std::find(usedIn.begin(), usedIn.end(), glyph);
		if(it != usedIn.end())
			usedIn.erase(it);
		else						
		{
			qDebug()<<"  removeFromUsedIn :  ERROR : that atom doesn't use already";		
			return 0;
		}
		
		return 1;
	}	

    inline int addNewNode(Node &currentNode)
    {
        if(nodes.empty())
        {
            if(currentNode.kind == Node::Move)
            {
                currentNode.kind = Node::On;
            }
            nodes.push_back(currentNode);
            return 0;
        }
        else if(nodeIsNotRedundantOneOrMove(currentNode))
        {
            nodes.push_back(currentNode);
        }

        return 1;
    }

    inline bool nodeKindIsMoveOrOn(const Node &node)
    {
        if((node.kind == Node::Move || node.kind == Node::On))
            return true;
        return false;
    }

    inline bool nodeIsNotRedundantOneOrMove(const Node &node)
    {
        if(!(nodeKindIsMoveOrOn((*prev(nodes.end()))) &&
             nodeKindIsMoveOrOn(node) &&
             (node.p.x == (*prev(nodes.end())).p.x && node.p.y == (*prev(nodes.end())).p.y)))
            return true;

        return false;
    }

    Ints glyphsFoundIn()
    {
        Ints glyphs;
        // добавляем в список те элементы которых в нем еще нет

        for(auto it = usedIn.begin(); it != usedIn.end(); it++)
        {
            if(std::find(glyphs.begin(), glyphs.end(), (*it)) == glyphs.end())
                glyphs.push_back((*it));
        }

        return glyphs;
    }


    Atom(): Contour()
    {
        rawAtomsCount = 1;
    }
	
	~Atom(){}	
};

class XNode // : public Node
{
public:
    Atom *atom;
    Matrix m;
    int reverse;    

    XNode(Atom &_contour, Matrix _m, int _reverse = -1):
        atom(&_contour),
        m(_m),
        reverse(_reverse)

    {}

    XNode(){}

};


typedef list<XNode> XNodes;

class XContour : public Contour
{
public:

    XNodes nodes;
};

typedef list<XContour> XContours;
typedef list<Atom> Atoms;

class DebugUtils
{
public:	
	
    static void coutMatrix(const string &header, const Matrix &m)
	{
		qDebug()<<header.c_str() << m.m11<< " "<< m.m12<< " "<< m.m21<< " "<< m.m22<< " "<< m.dx<< " " << m.dy; 			
	}
	
    static void nodesToDebug(const Nodes &nodes)
	{
		if(nodes.empty())
			qDebug() << "contour is empty!!!";
		
	  for (Nodes::const_iterator it = nodes.begin();  it != nodes.end(); ++it)
	    qDebug() << "Node::" << (*it).kind << ",Point(" << (*it).p.x << "," <<  (*it).p.y << ");";
	}
	
    static std::stringstream ctrToString(const Contour &contour)
	{	
		std::stringstream s_result;
		for (Nodes::const_iterator it = contour.nodes.begin(); it != contour.nodes.end(); ++it) 
		{
			if((*it).kind == 0)
			s_result << "		["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
			else
			s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
		}
		
		s_result<<"\n";		
		return	s_result;
	}	

    /*
    static std::stringstream atomsToString(PAContours &contours)
	{	
		std::stringstream s_result;		
		int cn = 0;
		for (PAContours::iterator it = contours.begin(); it != contours.end(); ++it, ++cn) 
		{			
			s_result << "	c# "<< cn << "\n";
			for (PositionedAtoms::iterator ai = (*it).begin(); ai != (*it).end(); ++ai) 
			{				
				PositionedAtom &a = (*ai);				
				s_result << " i: "<< a.contourIndex << 										
				"	M:  "<<"("<<a.m.m11<<","<<a.m.m12 <<","<< a.m.m21<<","<<a.m.m22<<","<<a.m.dx<<","<<a.m.dy<<")"<< 					
				" R: " << a.reverse << "\n";								 																	 			
			}			
		}			
		s_result<<"\n";		
		return	s_result;
	}

    */
	
};

class XGlyph
{
public:		
    XContours contours;
	string name;
	int index;	    
	
    XGlyph()
	{
		index = -1;						
	}
	    
    XGlyph(const XContours &_contours,string _name, int _index = -1) :
	contours(_contours),
	name(_name),
	index(_index)
	{					
	}	    
};

typedef list<XGlyph> XGlyphs;

class Representation
{
public:
    Matrix m;
    bool reverse;
    Splines s;
    int len;

    Representation(const Matrix &_m, bool _reverse, const Splines &_s, int _len) :
        m(_m),
        reverse(_reverse),
        s(_s),
        len(_len)
    {}
};

typedef list<Representation> Representations;

#endif // ATOM_H
