/*************************************************************************
*									 *
*	 YAP Prolog 	%W% %G% 					 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		TermExt.h						 *
* mods:									 *
* comments:	Extensions to standard terms for YAP			 *
* version:      $Id: TermExt.h,v 1.5 2006-01-02 02:25:45 vsc Exp $	 *
*************************************************************************/

#ifdef USE_SYSTEM_MALLOC
#define SF_STORE  (&(Yap_heap_regs->funcs))
#else
#define SF_STORE  ((special_functors *)HEAP_INIT_BASE)
#endif

#if USE_OFFSETS
#define   AtomFoundVar ((Atom)(&(((special_functors *)(NULL))->AtFoundVar)))
#define   AtomFreeTerm ((Atom)(&(((special_functors *)(NULL))->AtFreeTerm)))
#define   AtomNil ((Atom)(&(((special_functors *)(NULL))->AtNil)))
#define   AtomDot ((Atom)(&(((special_functors *)(NULL))->AtDot)))
#elif THREADS
#define   AtomFoundVar AbsAtom(SF_STORE->AtFoundVar)
#define   AtomFreeTerm AbsAtom(SF_STORE->AtFreeTerm)
#define   AtomNil AbsAtom(SF_STORE->AtNil)
#define   AtomDot AbsAtom(SF_STORE->AtDot)
#else
#define   AtomFoundVar AbsAtom(&(SF_STORE->AtFoundVar))
#define   AtomFreeTerm AbsAtom(&(SF_STORE->AtFreeTerm))
#define   AtomNil AbsAtom(&(SF_STORE->AtNil))
#define   AtomDot AbsAtom(&(SF_STORE->AtDot))
#endif

#define   TermFoundVar MkAtomTerm(AtomFoundVar)
#define   TermFreeTerm MkAtomTerm(AtomFreeTerm)
#define   TermNil MkAtomTerm(AtomNil)
#define   TermDot MkAtomTerm(AtomDot)

#if defined(IN_SECOND_QUADRANT) && !GC_NO_TAGS
typedef enum
{
  db_ref_e = sizeof (Functor *) | RBIT,
  long_int_e = 2 * sizeof (Functor *) | RBIT,
#ifdef USE_GMP
  big_int_e = 3 * sizeof (Functor *) | RBIT,
  double_e = 4 * sizeof (Functor *) | RBIT
#else
  double_e = 3 * sizeof (Functor *) | RBIT
#endif
}
blob_type;
#else
typedef enum
{
  db_ref_e = sizeof (Functor *),
  long_int_e = 2 * sizeof (Functor *),
#ifdef USE_GMP
  big_int_e = 3 * sizeof (Functor *),
  double_e = 4 * sizeof (Functor *)
#else
  double_e = 3 * sizeof (Functor *)
#endif
}
blob_type;
#endif

#define   FunctorDBRef    ((Functor)(db_ref_e))
#define   FunctorLongInt  ((Functor)(long_int_e))
#ifdef USE_GMP
#define   FunctorBigInt   ((Functor)(big_int_e))
#endif
#define   FunctorDouble   ((Functor)(double_e))
#define   EndSpecials     (double_e)


inline EXTERN blob_type BlobOfFunctor (Functor f);

inline EXTERN blob_type
BlobOfFunctor (Functor f)
{
  return (blob_type) ((CELL) f);
}



#ifdef COROUTINING

typedef struct
{
  /* what to do when someone tries to bind our term to someone else
     in some  predefined context */
  void (*bind_op) (Term *, Term);
  /* what to do if someone wants to copy our constraint */
  int (*copy_term_op) (CELL *, CELL ***, CELL *);
  /* copy the constraint into a term and back */
    Term (*to_term_op) (CELL *);
  int (*term_to_op) (Term, Term);
  /* op called to do marking in GC */
  void (*mark_op) (CELL *);
} ext_op;

/* known delays */
typedef enum
{
  empty_ext = 0 * sizeof (ext_op),	/* default op, this should never be called */
  attvars_ext = 1 * sizeof (ext_op)	/* support for attributed variables */
    /* add your own extensions here */
    /* keep this one */
}
exts;


/* array with the ops for your favourite extensions */
extern ext_op attas[attvars_ext + 1];

#endif

/* make sure that these data structures are the first thing to be allocated
   in the heap when we start the system */
#if THREADS
typedef struct special_functors_struct
{
  AtomEntry *AtFoundVar;
  AtomEntry *AtFreeTerm;
  AtomEntry *AtNil;
  AtomEntry *AtDot;
} special_functors;
#else
typedef struct special_functors_struct
{
  AtomEntry AtFoundVar;
  char AtFoundVarChars[8];
  AtomEntry AtFreeTerm;
  char AtFreeTermChars[8];
  AtomEntry AtNil;
  char AtNilChars[8];
  AtomEntry AtDot;
  char AtDotChars[8];
}
special_functors;
#endif

#if USE_SYSTEM_MALLOC
#define MAX_SPECIALS_TAG (4*4096)
#else
#define MAX_SPECIALS_TAG ((CELL)AtomBase)
#endif

#if SIZEOF_DOUBLE == SIZEOF_LONG_INT
#if GC_NO_TAGS

inline EXTERN Term MkFloatTerm (Float);

inline EXTERN Term
MkFloatTerm (Float dbl)
{
  return (Term) ((H[0] = (CELL) FunctorDouble, *(Float *) (H + 1) =
		  dbl, H[2] = (2 * sizeof (CELL) + EndSpecials), H +=
		  3, AbsAppl (H - 3)));
}


#else

inline EXTERN Term MkFloatTerm (Float);

inline EXTERN Term
MkFloatTerm (Float dbl)
{
  return (Term) ((H[0] = (CELL) FunctorDouble, *(Float *) (H + 1) =
		  dbl, H[2] = ((2 * sizeof (CELL) + EndSpecials) | MBIT), H +=
		  3, AbsAppl (H - 3)));
}


#endif


inline EXTERN Float FloatOfTerm (Term t);

inline EXTERN Float
FloatOfTerm (Term t)
{
  return (Float) (*(Float *) (RepAppl (t) + 1));
}



#define InitUnalignedFloat()

#else

#if SIZEOF_DOUBLE == 2*SIZEOF_LONG_INT

inline EXTERN Float STD_PROTO (CpFloatUnaligned, (CELL *));

inline EXTERN void STD_PROTO (AlignGlobalForDouble, (void));

#define DOUBLE_ALIGNED(ADDR) ((CELL)(ADDR) & 0x4)

#ifdef i386
inline EXTERN Float
CpFloatUnaligned (CELL * ptr)
{
  return *((Float *) (ptr + 1));
}

#else
/* first, need to address the alignment problem */
inline EXTERN Float
CpFloatUnaligned (CELL * ptr)
{
  union
  {
    Float f;
    CELL d[2];
  } u;
  u.d[0] = ptr[1];
  u.d[1] = ptr[2];
  return (u.f);
}

#endif

#if GC_NO_TAGS

inline EXTERN Term MkFloatTerm (Float);

inline EXTERN Term
MkFloatTerm (Float dbl)
{
  return (Term) ((AlignGlobalForDouble (), H[0] =
		  (CELL) FunctorDouble, *(Float *) (H + 1) = dbl, H[3] =
		  (3 * sizeof (CELL) + EndSpecials), H +=
		  4, AbsAppl (H - 4)));
}


#else

inline EXTERN Term MkFloatTerm (Float);

inline EXTERN Term
MkFloatTerm (Float dbl)
{
  return (Term) ((AlignGlobalForDouble (), H[0] =
		  (CELL) FunctorDouble, *(Float *) (H + 1) = dbl, H[3] =
		  ((3 * sizeof (CELL) + EndSpecials) | MBIT), H +=
		  4, AbsAppl (H - 4)));
}


#endif


inline EXTERN Float FloatOfTerm (Term t);

inline EXTERN Float
FloatOfTerm (Term t)
{
  return (Float) ((DOUBLE_ALIGNED (RepAppl (t)) ? *(Float *) (RepAppl (t) + 1)
		   : CpFloatUnaligned (RepAppl (t))));
}


/* no alignment problems for 64 bit machines */
#else
     /* OOPS, YAP only understands Floats that are as large as cells or that
        take two cells!!! */
#endif
#endif


inline EXTERN int IsFloatTerm (Term);

inline EXTERN int
IsFloatTerm (Term t)
{
  return (int) (IsApplTerm (t) && FunctorOfTerm (t) == FunctorDouble);
}




/* extern Functor FunctorLongInt; */
#if GC_NO_TAGS

inline EXTERN Term MkLongIntTerm (Int);

inline EXTERN Term
MkLongIntTerm (Int i)
{
  return (Term) ((H[0] = (CELL) FunctorLongInt, H[1] = (CELL) (i), H[2] =
		  (2 * sizeof (CELL) + EndSpecials), H +=
		  3, AbsAppl (H - 3)));
}


#else

inline EXTERN Term MkLongIntTerm (Int);

inline EXTERN Term
MkLongIntTerm (Int i)
{
  return (Term) ((H[0] = (CELL) FunctorLongInt, H[1] = (CELL) (i), H[2] =
		  ((2 * sizeof (CELL) + EndSpecials) | MBIT), H +=
		  3, AbsAppl (H - 3)));
}


#endif

inline EXTERN Int LongIntOfTerm (Term t);

inline EXTERN Int
LongIntOfTerm (Term t)
{
  return (Int) (RepAppl (t)[1]);
}



inline EXTERN int IsLongIntTerm (Term);

inline EXTERN int
IsLongIntTerm (Term t)
{
  return (int) (IsApplTerm (t) && FunctorOfTerm (t) == FunctorLongInt);
}




#ifdef USE_GMP
#include <stdio.h>
#include <gmp.h>


Term STD_PROTO (Yap_MkBigIntTerm, (MP_INT *));
Term STD_PROTO (Yap_MkBigIntTermCopy, (MP_INT *));
MP_INT *STD_PROTO (Yap_BigIntOfTerm, (Term));

inline EXTERN int IsBigIntTerm (Term);

inline EXTERN int
IsBigIntTerm (Term t)
{
  return (int) (IsApplTerm (t) && FunctorOfTerm (t) == FunctorBigInt);
}

inline EXTERN void MPZ_SET (mpz_t, MP_INT *);

inline EXTERN void
MPZ_SET (mpz_t dest, MP_INT *src)
{
  dest->_mp_size = src->_mp_size;
  dest->_mp_alloc = src->_mp_alloc;
  dest->_mp_d = src->_mp_d;
}

inline EXTERN int IsLargeIntTerm (Term);

inline EXTERN int
IsLargeIntTerm (Term t)
{
  return (int) (IsApplTerm (t)
		&& ((FunctorOfTerm (t) <= FunctorBigInt)
		    && (FunctorOfTerm (t) >= FunctorLongInt)));
}



#else


inline EXTERN int IsBigIntTerm (Term);

inline EXTERN int
IsBigIntTerm (Term t)
{
  return (int) (FALSE);
}




inline EXTERN int IsLargeIntTerm (Term);

inline EXTERN int
IsLargeIntTerm (Term t)
{
  return (int) (IsApplTerm (t) && FunctorOfTerm (t) == FunctorLongInt);
}



#endif

/* extern Functor FunctorLongInt; */

inline EXTERN int IsLargeNumTerm (Term);

inline EXTERN int
IsLargeNumTerm (Term t)
{
  return (int) (IsApplTerm (t)
		&& ((FunctorOfTerm (t) <= FunctorDouble)
		    && (FunctorOfTerm (t) >= FunctorLongInt)));
}




inline EXTERN int IsNumTerm (Term);

inline EXTERN int
IsNumTerm (Term t)
{
  return (int) ((IsIntTerm (t) || IsLargeNumTerm (t)));
}




inline EXTERN Int IsAtomicTerm (Term);

inline EXTERN Int
IsAtomicTerm (Term t)
{
  return (Int) (IsAtomOrIntTerm (t) || IsLargeNumTerm (t));
}




inline EXTERN Int IsExtensionFunctor (Functor);

inline EXTERN Int
IsExtensionFunctor (Functor f)
{
  return (Int) (f <= FunctorDouble);
}



inline EXTERN Int IsBlobFunctor (Functor);

inline EXTERN Int
IsBlobFunctor (Functor f)
{
  return (Int) ((f <= FunctorDouble && f >= FunctorDBRef));
}



inline EXTERN Int IsPrimitiveTerm (Term);

inline EXTERN Int
IsPrimitiveTerm (Term t)
{
  return (Int) ((IsAtomOrIntTerm (t)
		 || (IsApplTerm (t) && IsBlobFunctor (FunctorOfTerm (t)))));
}



#ifdef TERM_EXTENSIONS


inline EXTERN Int IsAttachFunc (Functor);

inline EXTERN Int
IsAttachFunc (Functor f)
{
  return (Int) (FALSE);
}




inline EXTERN Int IsAttachedTerm (Term);

inline EXTERN Int
IsAttachedTerm (Term t)
{
  return (Int) ((IsVarTerm (t) && VarOfTerm (t) < H0));
}




inline EXTERN Int SafeIsAttachedTerm (Term);

inline EXTERN Int
SafeIsAttachedTerm (Term t)
{
  return (Int) ((IsVarTerm (t) && VarOfTerm (t) < H0
		 && VarOfTerm (t) >= (CELL *) Yap_GlobalBase));
}




inline EXTERN exts ExtFromCell (CELL *);

inline EXTERN exts
ExtFromCell (CELL * pt)
{
  return attvars_ext;
}



#else


inline EXTERN Int IsAttachFunc (Functor);

inline EXTERN Int
IsAttachFunc (Functor f)
{
  return (Int) (FALSE);
}




inline EXTERN Int IsAttachedTerm (Term);

inline EXTERN Int
IsAttachedTerm (Term t)
{
  return (Int) (FALSE);
}




#endif

inline EXTERN int STD_PROTO (unify_extension, (Functor, CELL, CELL *, CELL));

EXTERN int STD_PROTO (unify_extension, (Functor, CELL, CELL *, CELL));

inline EXTERN int
unify_extension (Functor f, CELL d0, CELL * pt0, CELL d1)
{
  switch (BlobOfFunctor (f))
    {
    case db_ref_e:
      return (d0 == d1);
    case long_int_e:
      return (pt0[1] == RepAppl (d1)[1]);
#ifdef USE_GMP
    case big_int_e:
      return (mpz_cmp (Yap_BigIntOfTerm (d0), Yap_BigIntOfTerm (d1)) == 0);
#endif /* USE_GMP */
    case double_e:
      {
	CELL *pt1 = RepAppl (d1);
	return (pt0[1] == pt1[1]
#if SIZEOF_DOUBLE == 2*SIZEOF_LONG_INT
		&& pt0[2] == pt1[2]
#endif
	  );
      }
    }
  return (FALSE);
}
