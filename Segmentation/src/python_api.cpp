#include "python_api.h"


void init_python_api(void)
{
	printf("Calling Python Function\n");
	Py_Initialize();  // Initialize the Python interpreter.

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\".\")");

	PyObject *pName, *pModule, *pDict, *pFunc, *pArgs, *pValue;  // Create some Python objects that will later be assigned values.
	// Convert the file name to a Python string.
	pName = PyUnicode_FromString("3d_image_rendering");

	if (pName==NULL)
	   printf("filename is NULL\n"); 

	// Import the file as a Python module.
	pModule = PyImport_Import(pName);         
	if(pModule==NULL)
	{
	   printf("no Module\n");
	   PyErr_Print();
	}
	// Create a dictionary for the contents of the module.
	pDict = PyModule_GetDict(pModule);
	printf("After Dictionary retrieval\n");
	// Get the add method from the dictionary.
	pFunc = PyDict_GetItemString(pDict, "main");
	printf("Calling for the Python function!\n");
	PyObject_CallObject(pFunc, 0);
	printf("It's done!");
}
