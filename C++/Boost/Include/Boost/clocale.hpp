//////////////////////////////////////////////////////////////////////////
// {{{ VIDEOJET patch: missing locales on Windows CE
#pragma once

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4100 4127 4244)
#endif


#if !defined (BOOST_NO_STD_LOCALE) && !defined(UNDER_CE)
	#include <locale>
#else

#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <cstring>
#include <stdexcept>
#include <typeinfo>

namespace std
{
	class locale
	{
	public:
		static locale classic() { return locale(); }

		typedef int category;

		class id
		{
		public:
			id(size_t _Val = 0) : _Id(_Val) { }
			operator size_t() { return (_Id); }

		private:
			id(const id&);	// not defined
			id& operator=(const id&);	// not defined

			size_t _Id;	// the identifier stamp
		};

		class facet
		{
			friend class locale;
		public:
			static size_t _Getcat(const facet ** = 0, const locale * = 0)
			{
				return (size_t)(-1);
			}

			void _Register() { }
			virtual ~facet() { }

		protected:
			explicit facet(size_t _Initrefs = 0)
				: _Refs(_Initrefs)
			{	// construct with initial reference count
			}

		private:
			facet(const facet&);	// not defined
			facet& operator=(const facet&);	// not defined

			size_t _Refs;	// the reference count
		};
	}; // class locale

	class codecvt_base
	{
	public:
		enum { ok, partial, error, noconv };
		typedef int result;
	};

	template<class T1, class T2, class T3>
	class codecvt : public codecvt_base
	{
	public:
		codecvt() {}
		codecvt(size_t s){}
	};

#ifdef UNDER_CE
	inline bool (isalnum)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _ALPHA|_DIGIT); }
	inline bool (isalpha)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _ALPHA); }
	inline bool (iscntrl)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _CONTROL); }
	inline bool (isdigit)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _DIGIT); }
	inline bool (isgraph)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _PUNCT|_ALPHA|_DIGIT); }
	inline bool (islower)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _LOWER); }
	inline bool (isprint)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _BLANK|_PUNCT|_ALPHA|_DIGIT); }
	inline bool (ispunct)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _PUNCT); }
	inline bool (isspace)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _SPACE); }  
	inline bool (isupper)(wchar_t ch, const locale&)  { return 0 != iswctype(ch, _UPPER); }  
	inline bool (isxdigit)(wchar_t ch, const locale&) { return 0 != iswctype(ch, _HEX); }  

	template <typename TChar> TChar tolower(TChar, const locale&);
	template <typename TChar> TChar toupper(TChar, const locale&);

	inline char (tolower)(char ch, const locale&)  { return ::tolower(ch); }
	inline char (toupper)(char ch, const locale&)  { return ::toupper(ch); }
	inline wchar_t (tolower)(wchar_t ch, const locale&)  { return  towlower(ch); }
	inline wchar_t (toupper)(wchar_t ch, const locale&)  { return  towupper(ch); }

	inline int wctomb(char* _MbCh, wchar_t _WCh)   { return wcstombs(_MbCh, &_WCh, 1); }
	inline int mblen(const char* pCh, size_t _MaxCount)  { return (pCh == NULL || _MaxCount < 1) ? -1 : 1; }
	inline int mbtowc(wchar_t *pwc, const char *s, size_t n) { return mbstowcs(pwc, s, 1); }

	inline unsigned short btowc(char ch)
	{
		wchar_t w[] = {0xFFFD, 0, 0, 0, 0};
		mbtowc(w, &ch, 1);
		return w[0];
	}

#pragma warning(push)
#pragma warning(disable:4275)

#define _XA		0x100		/* extra alphabetic */
#define _XS		0x000		/* extra space */
#define _BB		_CONTROL	/* BEL, BS, etc. */
#define _CN		_SPACE		/* CR, FF, HT, NL, VT */
#define _DI		_DIGIT		/* '0'-'9' */
#define _LO		_LOWER		/* 'a'-'z' */
#define _PU		_PUNCT		/* punctuation */
#define _SP		_BLANK		/* space */
#define _UP		_UPPER		/* 'A'-'Z' */
#define _XD		_HEX		/* '0'-'9', 'A'-'F', 'a'-'f' */

		// STRUCT ctype_base
struct  ctype_base
	: public locale::facet
	{	// base for ctype
	enum
		{	// constants for character classifications
		alnum = _DI|_LO|_UP|_XA, alpha = _LO|_UP|_XA,
		cntrl = _BB, digit = _DI, graph = _DI|_LO|_PU|_UP|_XA,
		lower = _LO, print = _DI|_LO|_PU|_SP|_UP|_XA|_XD,
		punct = _PU, space = _CN|_SP|_XS, upper = _UP,
		xdigit = _XD};
	typedef short mask;	// to match <ctype.h>

	__CLR_OR_THIS_CALL ctype_base(size_t _Refs = 0)
		: locale::facet(_Refs)
		{	// default constructor
		}

	__CLR_OR_THIS_CALL ~ctype_base()
		{	// destroy the object
		}

_PROTECTED:
	static void __CLRCALL_OR_CDECL _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "out_of_range in ctype<T>");
		}
	};
#pragma warning(pop)
#endif

} // namespace std

#endif

#ifdef _WIN32
#pragma warning( pop )
#endif

// VIDEOJET patch }}}
//////////////////////////////////////////////////////////////////////////
