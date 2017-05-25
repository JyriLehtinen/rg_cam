#include "python_api.h"


void init_python_api(void)
{
	Py_Initialize();  // Initialize the Python interpreter.

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"../src/\")");

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
	//printf("After Dictionary retrieval\n");
	// Get the add method from the dictionary.
	pFunc = PyDict_GetItemString(pDict, "main_");

	pArgs = PyTuple_New(1);
    printf("\n\nNumber of arguments: %ld\n", PyTuple_Size(pArgs));
    const char* str = "Don't drop the ball!\n\n";
    pValue = PyByteArray_FromStringAndSize(str, strlen(str));
    if (!pValue)
    {
        Py_DECREF(pArgs);
        Py_DECREF(pModule);
        fprintf(stderr, "Cannot convert argument\n");
        return;
    }

	if(PyTuple_SetItem(pArgs, 0, pValue))
        printf("Tuple assignment fucked up :S\n");
    
    printf("This is what I'm passing to python: %s\n", PyByteArray_AsString(pValue));

	if(!PyObject_Call(pFunc, pArgs, NULL))
        fprintf(stderr, "\nCalling failed\n");
}
