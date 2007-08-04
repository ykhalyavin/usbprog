#include <stdio.h>    // to get "printf" function
#include <stdlib.h>   // to get "free" function
#include "xmlParser.h"

int main(int argc, char **argv)
{
  // this open and parse the XML file:
  XMLNode xMainNode=XMLNode::openFileHelper("PMMLModel.xml","PMML");

  // this prints "<Condor>":
  XMLNode xNode=xMainNode.getChildNode("Header");
  printf("Application Name is: '%s'\n", xNode.getChildNode("Application").getAttribute("name"));
  
  // this prints "Hello world!":
  printf("Text inside Header tag is :'%s'\n", xNode.getText());

  // this gets the number of "NumericPredictor" tags:
  xNode=xMainNode.getChildNode("RegressionModel").getChildNode("RegressionTable");
  int n=xNode.nChildNode("NumericPredictor");

  // this prints the "coefficient" value for all the "NumericPredictor" tags:
  for (int i=0; i<n; i++)
    printf("coeff%i=%f\n",i+1,atof(xNode.getChildNode("NumericPredictor",i).getAttribute("coefficient")));

  // this prints a formatted ouput based on the content of the first "Extension" tag of the XML file:
  char *t=xMainNode.getChildNode("Extension").createXMLString(true);
  printf("%s\n",t);
  free(t);
  return 0;
}
