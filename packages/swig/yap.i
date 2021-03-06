/* example.i */
%module(directors = "1") yap

// Language independent exception handler
%include exception.i
%include stdint.i

%ignore *::operator[];

class YAPPredicate;
class YAPEngine;

#define arity_t uintptr_t

#ifdef SWIGPYTHON

%typemap(typecheck) YAPTerm*  {
  $1 = PySequence_Check($input);
}

// Map a Python sequence into any sized C double array
%typemap(in) YAPTerm*  {
  int i;
  if (!PySequence_Check($input)) {
      PyErr_SetString(PyExc_TypeError,"Expecting a sequence");
      $1 = nullptr;
  } else {
  int sz = PyObject_Length($input);
  std::vector<YAPTerm> v(sz);
  for (i =0; i < sz; i++) {
      PyObject *o = PySequence_GetItem($input,i);
      v[i] = YAPTerm(pythonToYAP(o));
      Py_DECREF(o);
  }
  $1 = &v[0];
}
}

%typemap(typecheck) YAPTerm  {
 $1 = true;
}



%typemap(in) YAPTerm { $1 = YAPTerm(pythonToYAP($input)); }


%typemap(out) YAPTerm  {$result = term_to_python($1.handle(), false);}


%extend(out) YAPTerm{YAPTerm & __getitem__(size_t i){Term t0 = $self->term();

if (IsApplTerm(t0)) {
  Functor f = FunctorOfTerm(t0);
  if (!IsExtensionFunctor(f))
    return *new YAPTerm(ArgOfTerm(i + 1, t0));
} else if (IsPairTerm(t0)) {
  if (i == 0)
    return *new YAPTerm(HeadOfTerm(t0));
  else if (i == 1)
    return *new YAPTerm(TailOfTerm(t0));
	}
   }
}



// Language independent exception handler

%exception next {
  try {
    $action
  } catch (YAPError &e) {
    yap_error_number en = e.getID();
    PyObject *pyerr = PyExc_RuntimeError;

    LOCAL_Error_TYPE = YAP_NO_ERROR;
    switch (e.getErrorClass()) {
    case YAPC_NO_ERROR:
      break;
  /// bad domain, "first argument often is the predicate.
    case DOMAIN_ERROR: {
      switch (en) {
      case DOMAIN_ERROR_OUT_OF_RANGE:
      case DOMAIN_ERROR_NOT_LESS_THAN_ZERO:
        pyerr = PyExc_IndexError;
        break;
      case DOMAIN_ERROR_CLOSE_OPTION:
      case DOMAIN_ERROR_ENCODING:
      case DOMAIN_ERROR_PROLOG_FLAG:
      case DOMAIN_ERROR_ABSOLUTE_FILE_NAME_OPTION:
      case DOMAIN_ERROR_READ_OPTION:
      case DOMAIN_ERROR_SET_STREAM_OPTION:
        pyerr = PyExc_KeyError;
        break;
      case DOMAIN_ERROR_FILE_ERRORS:
      case DOMAIN_ERROR_FILE_TYPE:
      case DOMAIN_ERROR_IO_MODE:
      case DOMAIN_ERROR_SOURCE_SINK:
      case DOMAIN_ERROR_STREAM_POSITION:
        pyerr = PyExc_IOError;
        break;
      default:
        pyerr = PyExc_ValueError;
      }
    } break;
    /// bad arithmetic
    case EVALUATION_ERROR: {
      switch (en) {
      case EVALUATION_ERROR_FLOAT_OVERFLOW:
      case EVALUATION_ERROR_INT_OVERFLOW:
        pyerr = PyExc_OverflowError;
        break;
      case EVALUATION_ERROR_FLOAT_UNDERFLOW:
      case EVALUATION_ERROR_UNDERFLOW:
      case EVALUATION_ERROR_ZERO_DIVISOR:
        pyerr = PyExc_ArithmeticError;
        break;
      default:
        pyerr = PyExc_RuntimeError;
      }
    } break;
    /// missing object (I/O mostly)
    case EXISTENCE_ERROR:
      pyerr = PyExc_NotImplementedError;
      break;
    /// should be bound
    case INSTANTIATION_ERROR_CLASS:
      pyerr = PyExc_RuntimeError;
      break;
    /// bad access, I/O
    case PERMISSION_ERROR: {
      switch (en) {
      case PERMISSION_ERROR_INPUT_BINARY_STREAM:
      case PERMISSION_ERROR_INPUT_PAST_END_OF_STREAM:
      case PERMISSION_ERROR_INPUT_STREAM:
      case PERMISSION_ERROR_INPUT_TEXT_STREAM:
      case PERMISSION_ERROR_OPEN_SOURCE_SINK:
      case PERMISSION_ERROR_OUTPUT_BINARY_STREAM:
      case PERMISSION_ERROR_REPOSITION_STREAM:
      case PERMISSION_ERROR_OUTPUT_STREAM:
      case PERMISSION_ERROR_OUTPUT_TEXT_STREAM:
        pyerr = PyExc_OverflowError;
        break;
      default:
        pyerr = PyExc_RuntimeError;
      }
    } break;
    /// something that could not be represented into a type
    case REPRESENTATION_ERROR:
      pyerr = PyExc_RuntimeError;
      break;
    /// not enough ....
    case RESOURCE_ERROR:
      pyerr = PyExc_RuntimeError;
      break;
    /// bad text
    case SYNTAX_ERROR_CLASS:
      pyerr = PyExc_SyntaxError;
      break;
    /// OS or internal
    case SYSTEM_ERROR_CLASS:
      pyerr = PyExc_RuntimeError;
      break;
    /// bad typing
    case TYPE_ERROR:
      pyerr = PyExc_TypeError;
      break;
    /// should be unbound
    case UNINSTANTIATION_ERROR_CLASS:
      pyerr = PyExc_RuntimeError;
      break;
    /// escape hatch
    default:
      break;
    }
    PyErr_SetString(pyerr, e.text());
   }
}

#else

// Language independent exception handler
%include exception.i

    %exception {
  try {
    $action
  } catch (YAPError e) {
yap_error_number en = e.getID();
LOCAL_Error_TYPE = YAP_NO_ERROR;
    switch (e.getErrorClass()) {
    case YAPC_NO_ERROR:
      break;
    /// bad domain, "first argument often is the predicate.
    case DOMAIN_ERROR: {
      switch (en) {
      case DOMAIN_ERROR_OUT_OF_RANGE:
      case DOMAIN_ERROR_NOT_LESS_THAN_ZERO:
        SWIG_exception(SWIG_IndexError, e.text());
        break;
      case DOMAIN_ERROR_CLOSE_OPTION:
      case DOMAIN_ERROR_ENCODING:
      case DOMAIN_ERROR_PROLOG_FLAG:
      case DOMAIN_ERROR_ABSOLUTE_FILE_NAME_OPTION:
      case DOMAIN_ERROR_READ_OPTION:
      case DOMAIN_ERROR_SET_STREAM_OPTION:
        SWIG_exception(SWIG_AttributeError, e.text());
        break;
      case DOMAIN_ERROR_FILE_ERRORS:
      case DOMAIN_ERROR_FILE_TYPE:
      case DOMAIN_ERROR_IO_MODE:
      case DOMAIN_ERROR_SOURCE_SINK:
      case DOMAIN_ERROR_STREAM_POSITION:
        SWIG_exception(SWIG_IOError, e.text());
        break;
      default:
        SWIG_exception(SWIG_ValueError, e.text());
      }
    } break;
    /// bad arithmetic
    case EVALUATION_ERROR: {
      switch (en) {
      case EVALUATION_ERROR_FLOAT_OVERFLOW:
      case EVALUATION_ERROR_FLOAT_UNDERFLOW:
      case EVALUATION_ERROR_INT_OVERFLOW:
      case EVALUATION_ERROR_UNDERFLOW:
        SWIG_exception(SWIG_OverflowError, e.text());
        break;
      case EVALUATION_ERROR_ZERO_DIVISOR:
        SWIG_exception(SWIG_DivisionByZero, e.text());
        break;
      default:
        SWIG_exception(SWIG_RuntimeError, e.text());
      }
    } break;
    /// missing object (I/O mostly)
    case EXISTENCE_ERROR:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// should be bound
    case INSTANTIATION_ERROR_CLASS:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// bad access, I/O
    case PERMISSION_ERROR: {
      switch (en) {
      case PERMISSION_ERROR_INPUT_BINARY_STREAM:
      case PERMISSION_ERROR_INPUT_PAST_END_OF_STREAM:
      case PERMISSION_ERROR_INPUT_STREAM:
      case PERMISSION_ERROR_INPUT_TEXT_STREAM:
      case PERMISSION_ERROR_OPEN_SOURCE_SINK:
      case PERMISSION_ERROR_OUTPUT_BINARY_STREAM:
      case PERMISSION_ERROR_REPOSITION_STREAM:
      case PERMISSION_ERROR_OUTPUT_STREAM:
      case PERMISSION_ERROR_OUTPUT_TEXT_STREAM:
        SWIG_exception(SWIG_OverflowError, e.text());
        break;
      default:
        SWIG_exception(SWIG_RuntimeError, e.text());
      }
    } break;
    /// something that could not be represented into a type
    case REPRESENTATION_ERROR:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// not enough ....
    case RESOURCE_ERROR:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// bad text
    case SYNTAX_ERROR_CLASS:
      SWIG_exception(SWIG_SyntaxError, e.text());
      break;
    /// OS or internal
    case SYSTEM_ERROR_CLASS:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// bad typing
    case TYPE_ERROR:
      SWIG_exception(SWIG_TypeError, e.text());
      break;
    /// should be unbound
    case UNINSTANTIATION_ERROR_CLASS:
      SWIG_exception(SWIG_RuntimeError, e.text());
      break;
    /// escape hatch
    default:
      break;
    }
  }
}

#endif

%{
/* Put header files here or function declarations like below */

#define YAP_CPP_INTERFACE 1

#include "yapi.hh"

        extern "C" {

#if THREADS
#define Yap_regp regcache
#endif

            // we cannot consult YapInterface.h, that conflicts with what we
            // declare, though
            // it shouldn't
        }

%}

    /* turn on director wrapping Callback */
%feature("director") YAPCallback;

// %include "yapi.hh"

%include "yapa.hh"

    %include "yapie.hh"

    %include "yapt.hh"

    %include "yapdb.hh"

    %include "yapq.hh"

%init %{
#ifdef SWIGPYTHON
  init_python();
#endif
%}
