#include <Python.h>
#include "structmember.h"
#include "../libsrs2/srs2.h"

typedef struct {
	PyObject_HEAD
	srs_t	*srs;
} Libsrs2Object;

static PyObject *
Libsrs2_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Libsrs2Object	*self;

	self = (Libsrs2Object *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->srs = srs_new();
		/* Add a secret */
	}

	return (PyObject *)self;
}

static PyObject *
Libsrs2_init(Libsrs2Object *self, PyObject *args, PyObject *kwds)
{
}

static PyObject *
Libsrs2_free(Libsrs2Object *self, PyObject *args)
{
	srs_free(self->srs);
}

static PyObject *
Libsrs2_forward(Libsrs2Object *self, PyObject *args)
{
	char	*sender;
	char	*alias;

	if (!PyAry_ParseTuple(args, "ss", &sender, &alias))
		return NULL;
	return Py_BuildValue("i", SRS_SUCCESS);
}

static PyObject *
Libsrs2_reverse(Libsrs2Object *self, PyObject *args)
{
	char	*sender;

	if (!PyAry_ParseTuple(args, "s", &sender))
		return NULL;
	return Py_BuildValue("i", SRS_SUCCESS);
}

static PyMethodDef Libsrs2Methods[] = {
	{"forward",  (PyCFunction)Libsrs2_forward, METH_VARARGS,
					"Forward-rewrite an address."},
	{"reverse",  (PyCFunction)Libsrs2_reverse, METH_VARARGS,
					"Reverse-rewrite an address."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyMemberDef Libsrs2Members[] = {
	{NULL}  /* Sentinel */
};

static PyTypeObject Libsrs2Type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"libsrs2.Libsrs2",         /*tp_name*/
	sizeof(Libsrs2Object), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Libsrs2_free,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	"Libsrs2 handle",          /* tp_doc */
	0,		                   /* tp_traverse */
	0,    		               /* tp_clear */
	0,	    	               /* tp_richcompare */
	0,	    	               /* tp_weaklistoffset */
	0,	    	               /* tp_iter */
	0,	    	               /* tp_iternext */
	Libsrs2Methods,            /* tp_methods */
	Libsrs2Members,            /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)Libsrs2_init,    /* tp_init */
	0,                         /* tp_alloc */
	Libsrs2_new,               /* tp_new */
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initlibsrs2(void)
{
	PyObject	*m;
	Libsrs2Type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&Libsrs2Type) < 0)
		return;
	m = Py_InitModule3("libsrs2", Libsrs2Methods,
					"Interface to libsrs2 (http://www.libsrs2.org/)");
	Py_INCREF(&Libsrs2Type);
	PyModule_AddObject(m, "Libsrs2", (PyObject *)&Libsrs2Type);
}
