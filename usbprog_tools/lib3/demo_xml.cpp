#include <stdio.h>    // to get "printf" function
#include <stdlib.h>   // to get "free" function
#include "xmlParser.h"

int main(int argc, char **argv)
{
  // this open and parse the XML file:
  XMLNode xMainNode=XMLNode::openFileHelper("versions.xml","usbprog");

  printf("Application Name is: '%s'\n", xMainNode.getChildNode("application").getAttribute("name"));
  printf("Application Version is: '%s'\n", xMainNode.getChildNode("application").getAttribute("version"));
  
  XMLNode xNode=xMainNode.getChildNode("pool");
  int n=xNode.nChildNode("firmware");
  for (int i=0; i<n; i++){
    printf("firmware: %s\n",xNode.getChildNode("firmware",i).getAttribute("name"));
    printf("label: %s\n",xNode.getChildNode("firmware",i).getAttribute("label"));
    printf("file: %s\n",xNode.getChildNode("firmware",i).getChildNode("binary").getAttribute("file"));
  }

  return 0;
}
