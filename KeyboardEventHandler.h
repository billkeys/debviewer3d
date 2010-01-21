#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H
#include <iostream>
#include <osgGA/GUIEventHandler>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

   typedef void (*FunctionType) ();

   enum KeyStatusType { KEY_UP, KEY_DOWN };

   struct FunctionStatusType
   {
      FunctionStatusType() { keyState = KEY_UP; keyFunction = NULL; }
      FunctionType keyFunction;
      KeyStatusType keyState;
   };

   bool addFunction(int whatKey, FunctionType newFunction);

   bool addFunction(int whatKey, KeyStatusType keyPressStatus, FunctionType newFunction);

   virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

   virtual void accept(osgGA::GUIEventHandlerVisitor& v)   { v.visit(*this); };

protected:

   typedef std::map<int, FunctionStatusType > KeyFunctionMap;
   KeyFunctionMap keyFuncMap;
   KeyFunctionMap keyUPFuncMap;

};

#endif

