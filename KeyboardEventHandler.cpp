#include "KeyboardEventHandler.h"

bool KeyboardEventHandler::addFunction(int whatKey, FunctionType newFunction)
{
   if ( keyFuncMap.end() != keyFuncMap.find( whatKey ))
   {
      std::cout << "duplicate key '" << whatKey << "' ignored." << std::endl;
      return false;
   }
   else
   {
      keyFuncMap[whatKey].keyFunction = newFunction;
      return true;
   }
}

bool KeyboardEventHandler::addFunction (int whatKey, KeyStatusType keyPressStatus, FunctionType newFunction)
{
   if (keyPressStatus == KEY_DOWN)
   {
      return addFunction(whatKey,newFunction);
   }
   else
   {
      if ( keyUPFuncMap.end() != keyUPFuncMap.find( whatKey )) 
      {
         std::cout << "duplicate key '" << whatKey << "' ignored." << std::endl;
         return false;
      }
      else
      {
         keyUPFuncMap[whatKey].keyFunction = newFunction;
         return true;
      }
   } // KEY_UP
}

bool KeyboardEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
   bool newKeyDownEvent = false;
   bool newKeyUpEvent   = false;

   switch(ea.getEventType())
   {
   case(osgGA::GUIEventAdapter::KEYDOWN):
      {
         KeyFunctionMap::iterator itr = keyFuncMap.find(ea.getKey());
         if (itr != keyFuncMap.end())
         {
            if ( (*itr).second.keyState == KEY_UP )
            {
               (*itr).second.keyState = KEY_DOWN;
               newKeyDownEvent = true;
            }
            if (newKeyDownEvent)
            {
               (*itr).second.keyFunction();
               newKeyDownEvent = false;
            }
            return true;
         }
         return false;
      }
   case(osgGA::GUIEventAdapter::KEYUP):
      {
         KeyFunctionMap::iterator itr = keyFuncMap.find(ea.getKey());
         if (itr != keyFuncMap.end() )
         {
            (*itr).second.keyState = KEY_UP;
         }
         itr = keyUPFuncMap.find(ea.getKey());
         if (itr != keyUPFuncMap.end())
         {
            (*itr).second.keyFunction();
            return true;
         }
         return false; 
      }
   default:
      return false;
   }
}

