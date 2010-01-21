/*
 * Debviewer3d.cpp - Visualizes Dependencies of deb packages
 *
 * Overview: 
 *           Displays a 3D graph with dependencies of a defined Ubuntu package. 
 *           Also lets the user interact and get more information about 
 *           the displayed packages.                       
 *
 * Usage:
 *        ./debviewer3d <packageName>
 *	    
 *        If no package is defined the default openscenegraph package will be displayed.
 *	  Click on the name of package to display more information then click on it again
 *	  to toggle it back.
 *        The user can also use the mouse to change the view of the display.
 *
 *        The color of each package depends on the section type of the package:
 *          libs = red, editors = green, python = white, unknown = black,
 *          web = purple,graphics = yellow, interpreters = blue, universe orange
 *       
 *        Press q to exit
 *
 * Example:
 * 	    ./debviewer3d vim
 *
 * Author: William G. Keys
 * Created: 7 Nov 2009
 * Modified: 20 Dec 2009
 */

#include <stdlib.h> 
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstring> 
#include <set> 
#include <list> 
#include <osg/Group>
#include <osg/Node>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/NodeTrackerManipulator>
#include <osgSim/DOFTransform>
#include <osgText/Font>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/io_utils>
#include <osg/observer_ptr>
#include <osgUtil/PolytopeIntersector>
#include <osg/NodeCallback>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Label>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>
#include <osgUtil/LineSegmentIntersector>
	
#include <boost/regex.hpp>

#include "KeyboardEventHandler.h"
#include "Package.h"

using namespace std;
using namespace boost;

/* 
  Mouse handler
*/
osg::ref_ptr<osg::Node> _selectedNode;
osg::ref_ptr<osgText::Text> textNew;

Package pack;
osg::ref_ptr<osg::Node> rootNode;
const unsigned int MASK_3D = 0x0F000000;

/* 
  Creates the Widget key
*/
class Key: public osgWidget::Box {
    osg::ref_ptr<osgWidget::Box>    _tabs;
    osg::ref_ptr<osgWidget::Canvas> _windows;

public:
    Key(const std::string& name):
    osgWidget::Box(name, osgWidget::Box::VERTICAL) {
        _windows = new osgWidget::Canvas("canvas");

        std::stringstream ss;
        
	// Setting up the window widget 
        std::stringstream descr;

        descr
	      << "Section          Color \n" << std::endl
	      << "Universe       = Orange" << std::endl
	      << "Libs           = Red" << std::endl
	      << "Editors        = Green" << std::endl
	      << "Python         = White" << std::endl
	      << "Web            = Purple"<< std::endl
	      << "Graphics       = Yellow" << std::endl
	      << "Interpreters   = Blue" << std::endl 
	      << "Unknown        = Black" << std::endl 
         ;

         osgWidget::Label* label1 = new osgWidget::Label(ss.str());

         label1->setFontSize(17);
         label1->setFontColor(0.0f, 0.0f, 0.0f, 1.0f);
         label1->setColor(1.0f, 1.0f, 1.0f, 1.0f);
	 label1->setLabel(descr.str()); 
         label1->addSize(50.0f, 50.0f);
         _windows->addWidget(label1, 0.0f, 0.0f);

        addWidget(_windows->embed());
    }
};

// PickHandler -- A GUIEventHandler that implements picking.
class PickHandler : public osgGA::GUIEventHandler
{
public:

    PickHandler() : _mX( 0. ),_mY( 0. ) {}
    bool handle( const osgGA::GUIEventAdapter& ea,
            osgGA::GUIActionAdapter& aa )
    {
        osgViewer::Viewer* viewer =
                dynamic_cast<osgViewer::Viewer*>( &aa );
        if (!viewer)
            return( false );

        switch( ea.getEventType() )
        {
            case osgGA::GUIEventAdapter::PUSH:
            case osgGA::GUIEventAdapter::MOVE:
            {
                // Record mouse location for the button press
                //   and move events.
                _mX = ea.getX();
                _mY = ea.getY();
                return( false );
            }
            case osgGA::GUIEventAdapter::RELEASE:
            {
                // If the mouse hasn't moved since the last
                //   button press or move event, perform a
                //   pick. (Otherwise, the trackball
                //   manipulator will handle it.)
                if (_mX == ea.getX() && _mY == ea.getY())
                {
                    if (pick( ea.getXnormalized(),ea.getYnormalized(), viewer ))
                      return( true );
                }
                return( false );
            }

            default:
                return( false );
        }
    }

   protected:
    // Store mouse xy location for button press & move events.
    float _mX, _mY;
    // Perform a pick operation.
    bool pick( const double x, const double y,
            osgViewer::Viewer* viewer )
    {
        if (!viewer->getSceneData())
            // Nothing to pick.
            return( false );

        double w( .05 ), h( .05 );
        osgUtil::PolytopeIntersector* picker =
          new osgUtil::PolytopeIntersector(osgUtil::Intersector::PROJECTION,x-w, y-h, x+w, y+h );

        osgUtil::IntersectionVisitor iv( picker );
        viewer->getCamera()->accept( iv );

        if (picker->containsIntersections())
        {
            const osg::NodePath& nodePath =
                    picker->getFirstIntersection().nodePath;
            unsigned int idx = nodePath.size();

            while (idx--)
            {

                // Find the LAST MatrixTransform in the node
                //   path; this will be the MatrixTransform
                //   to attach our callback to.
                 osg::Geode* mt =
                        dynamic_cast<osg::Geode*>( nodePath[ idx ] );

                if (mt == NULL)
                    continue;

                _selectedNode = mt;
                
		// If we get here, we just found a
                if (_selectedNode.valid()) {
                    // Clear the previous selected node's
                    
		    string pname = mt->getName();

                    if (!pname.empty()) {
                       Package package_class = pack.get_package_info(pname);
		    
                       osgText::Text* text = dynamic_cast<osgText::Text*>(mt->getDrawable(0));

		       string newText = package_class.version;
		       osgText::String ctext  = text->getText();

		       // Check if we need to set it back to package name
                       if ( ctext.createUTF8EncodedString()  == package_class.name) {
                          string newText  = "Section: ";
			  newText.append(package_class.section);
			  newText.append("\nSize: ");
			  newText.append(package_class.size);
			  newText.append(" B");
			  newText.append("\nVersion: ");
			  newText.append(package_class.version);

                          text->setColor( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.f ) );
                          text->setText(newText);
		       }
                       else {
                         text->setColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                         text->setText(package_class.name);
		       }
                    }
                }
                break;
            }
            if (!_selectedNode.valid()){
                osg::notify() << "Pick failed." << std::endl;
            }
        }
        else if (_selectedNode.valid())
        {
            _selectedNode->setUpdateCallback( NULL );
            _selectedNode = NULL;
        }
        return( _selectedNode.valid() );
    }
};

void usage() {
       cout <<  "\nUsage:\n Click on a package name to get more information.\n" << endl;
}

/* 
 Callback functions
*/
void quit_func()
{
   exit(0);
}

/* 
 Model functions
*/
osg::ref_ptr<osg::PositionAttitudeTransform>  createLine(float x1, float y1, float z1, float x2, float y2, float z2)
{
  // Create an object to store geometry in.
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::ref_ptr<osg::PositionAttitudeTransform> lineNodePAT
        = new osg::PositionAttitudeTransform;

  // Create an array of four vertices.
  osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
  geom->setVertexArray( v.get() );
  v->push_back( osg::Vec3( x1, y1, z1 ) );
  v->push_back( osg::Vec3( x2, y2, z2 ) );
 
  // Create an array of a color.
  osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
  geom->setColorArray( c.get() );
  geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
  c->push_back( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );

  // Draw a four-vertex quad from the stored data.
  geom->addPrimitiveSet(
     new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 2 ) );

  // Add the Geometry (Drawable) to a Geode and add it as a child of root
  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  geode->addDrawable( geom.get() );
  lineNodePAT->addChild(geode);

   return lineNodePAT;
}

// Creates the text for the scene graph
osg::ref_ptr<osg::PositionAttitudeTransform> createText(string pname, osg::ref_ptr<osgText::Font> font, float x, float y, float z)
{
  osg::ref_ptr<osgText::Text> text = new osgText::Text;
  osg::ref_ptr<osg::PositionAttitudeTransform> textNodePAT
        = new osg::PositionAttitudeTransform;
  text->setFont( font.get() );
  text->setText(pname);
  
  text->setPosition( osg::Vec3( x, y, z) );
  text->setAxisAlignment( osgText::Text::SCREEN );
  text->setCharacterSize( 0.1f );
  
  text->setFontResolution( 128, 128 );
  text->setColor( osg::Vec4( 1.f, 1.f, 1.f, 1.f ) );

  // Add the Geometry (Drawable) to a Geode and add it as a child of root
  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  geode->setName(pname);  // Used to id the node for picking
  
  geode->addDrawable( text.get() );
  textNodePAT->addChild(geode);

  return textNodePAT;
} 

// creates the Package node for the scene graph
osg::PositionAttitudeTransform* createPackage(string pname, float x, float y, float z, string color)
{

  //Defining a Font node which will be used as the font for all the strings
  osg::ref_ptr<osgText::Font> font =
          osgText::readFontFile( "fonts/Arcan.ttf" );
  osg::ref_ptr<osg::PositionAttitudeTransform> textNode = createText(pname,font,x,y,z+.1); 
  
  textNode->setPosition(osg::Vec3(-x,-y,-z));  

  // Create a sphere centered at the origin, unit radius:
  osg::Sphere* unitSphere		 = new osg::Sphere( osg::Vec3(0,0,0), .05);
  osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(unitSphere);
  
  if (color == "red")
     unitSphereDrawable->setColor( osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );
  else if (color == "green")
     unitSphereDrawable->setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
  else if (color == "blue")
     unitSphereDrawable->setColor(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
  else if (color == "white")
     unitSphereDrawable->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
  else if (color == "purple")
     unitSphereDrawable->setColor(osg::Vec4(1.0f, 0.0f, 1.0f, 1.0f));
  else if (color == "yellow")
     unitSphereDrawable->setColor(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
  else if (color == "orange")
     unitSphereDrawable->setColor(osg::Vec4(1.0f, .5f, 0.0f, 1.0f));
  else
     unitSphereDrawable->setColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));

  osg::PositionAttitudeTransform* sphereXForm =   new osg::PositionAttitudeTransform();

  sphereXForm->setPosition(osg::Vec3(x,y,z));
  osg::Geode* unitSphereGeode = new osg::Geode();

  sphereXForm->addChild(unitSphereGeode);
  unitSphereGeode->addDrawable(unitSphereDrawable);
  sphereXForm->addChild(textNode);
    
  return sphereXForm; 
}

int main(int argc, char* argv[]) {
  set<string>::const_iterator cur_package;

  // Defining colors for the packages
  typedef map<string, string> mapType;
  mapType colorNode;
  colorNode["unknown"]	= "black";
  colorNode["libs"]	= "red";
  colorNode["editors"]	= "green";
  colorNode["python"]	= "white";
  colorNode["web"]	= "purple";
  colorNode["graphics"]	= "yellow";
  colorNode["interpreters"] = "blue";
  colorNode["universe"] = "orange";
  
  regex reSection("universe.*");
  cmatch matches;

  typedef struct {
    float x;
    float y;
    float z;
  }CIRCLE;
 
  vector<Package> package_list;
 
  // Error checking
  if (argc == 1)
  {
      char defaultPackage[] = "openscenegraph";
      cout <<  "\n Displaying the default openscenegraph package\n" << endl;
      argv[1] = defaultPackage;
      argc++;
  }

  string root_package = argv[1];

  list< pair<string,string> > list_dep; 

  // processing user command line input
  for(int i = 1; i < argc; i++) 
  {
    list_dep		= pack.get_package_dep(argv[i]);

    // Error checking
    if (list_dep.size() == 0){
       cout <<  "\nUsage:\n Please define a Ubuntu Package that has dependencies!\n" << endl;
       cout <<  "\n  Package_Dep <package name>\n" << endl;
       exit(1);  
    }
  }
  usage();

  osg::ref_ptr<osg::Group> root = new osg::Group();

  // root package
  Package package_class	= pack.get_package_info(root_package);

  // making sure we set the color for all the universe packages
  if (regex_match(package_class.section.c_str(), matches, reSection)) 
     package_class.section = "universe";
  
  string section = package_class.section;
     
  osg::PositionAttitudeTransform* package = createPackage(root_package, 0, 0, 0,colorNode[section]);

  // radius depends on the number of dependences
  float diameter = list_dep.size() * .02;

  float r = 3.1416 * diameter;  // computing the radius

  CIRCLE circle;
  circle.z = r; // defining how far the depences are from the root 

  list< pair<string,string> > ::const_iterator it;

  int num_dep_count = 0;

  // Creating the scene graph by interating through depedences
  for (it=list_dep.begin(); it != list_dep.end(); ++it) 
  {
     circle.x = r * cos(num_dep_count);
     circle.y = r * sin(num_dep_count);
  
     // add the Line node to the root 
     osg::ref_ptr<osg::PositionAttitudeTransform> lineNode = createLine(0.0, 0.0, 0.0, circle.y, circle.z, circle.x);
     
     Package package_class = pack.get_package_info(it->second);
     osg::PositionAttitudeTransform* package_dep;
    
     // making sure we set the color for all the universe packages
     if (regex_match(package_class.section.c_str(), matches, reSection)) 
        package_class.section = "universe";

     package_dep =  createPackage(it->second, circle.y, circle.z, circle.x,colorNode[package_class.section]);
     
     lineNode->addChild(package_dep);
     package->addChild(lineNode.get());
     num_dep_count++;
  }
  root->addChild(package);
  
  // defining our keyboard invent handlers
  KeyboardEventHandler* keyHandler = new KeyboardEventHandler();
  keyHandler->addFunction('q', quit_func);

  // setting up the viewer
  osgViewer::Viewer viewer;
  viewer.getCamera()->setClearColor( osg::Vec4( 0., .1, 1.0, 1. ) );
  
  // handlers provide useful interactive functions
  viewer.addEventHandler(new osgViewer::StatsHandler);
  viewer.addEventHandler(new osgViewer::WindowSizeHandler);
  viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
  viewer.getEventHandlers().push_front(keyHandler); 
  
  // add pick handler
  viewer.addEventHandler( new PickHandler );
  
  viewer.setSceneData(root);

  // adding the Key widget
  osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        720.0f,
        MASK_3D //,
        //osgWidget::WindowManager::WM_USE_RENDERBINS
    );

  Key* key = new Key("key");
  key->attachMoveCallback();

  wm->addChild(key);
    
  return osgWidget::createExample(viewer, wm,root);
} 
