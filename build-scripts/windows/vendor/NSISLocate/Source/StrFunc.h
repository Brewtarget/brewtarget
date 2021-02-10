/*****************************************************************
 *              String functions header v5.0                     *
 *                                                               *
 * 2011 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *                                                               *
 *                                                               *
 *Functions:                                                     *
 * WideCharLower, WideCharUpper, xmemcpy, xmemcmp, xmemset,      *
 * xarraysizeA, xarraysizeW, xstrlenA, xstrlenW,                 *
 * xstrcmpA, xstrcmpW, xstrcmpiA, xstrcmpiW,                     *
 * xstrcmpnA, xstrcmpnW, xstrcmpinA, xstrcmpinW,                 *
 * xstrcpyA, xstrcpyW, xstrcpynA, xstrcpynW,                     *
 * xstrstrA, xstrstrW, xstrrepA, xstrrepW                        *
 *                                                               *
 * xatoiA, xatoiW, xatoi64A, xatoi64W,                           *
 * xitoaA, xitoaW, xuitoaA, xuitoaW, xi64toaA, xi64toaW,         *
 * hex2decA, hex2decW, dec2hexA, dec2hexW,                       *
 * bin2hexA, bin2hexW, hex2binA, hex2binW, xprintfA, xprintfW    *
 *                                                               *
 * UTF16toUTF8, UTF8toUTF16, UTF32toUTF16, UTF16toUTF32          *
 *****************************************************************/

#ifndef _STRFUNC_H_
#define _STRFUNC_H_

wchar_t WideCharLower(wchar_t c);
wchar_t WideCharUpper(wchar_t c);
void* xmemcpy(void *dest, const void *src, UINT_PTR count);
int xmemcmp(const void *buf1, const void *buf2, UINT_PTR count);
void* xmemset(void *dest, int c, UINT_PTR count);
INT_PTR xarraysizeA(const char *pString, int *nElements);
INT_PTR xarraysizeW(const wchar_t *wpString, int *nElements);
INT_PTR xstrlenA(const char *pString);
INT_PTR xstrlenW(const wchar_t *wpString);
int xstrcmpA(const char *pString1, const char *pString2);
int xstrcmpW(const wchar_t *wpString1, const wchar_t *wpString2);
int xstrcmpiA(const char *pString1, const char *pString2);
int xstrcmpiW(const wchar_t *wpString1, const wchar_t *wpString2);
int xstrcmpnA(const char *pString1, const char *pString2, UINT_PTR dwMaxLength);
int xstrcmpnW(const wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength);
int xstrcmpinA(const char *pString1, const char *pString2, UINT_PTR dwMaxLength);
int xstrcmpinW(const wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength);
INT_PTR xstrcpyA(char *pString1, const char *pString2);
INT_PTR xstrcpyW(wchar_t *wpString1, const wchar_t *wpString2);
INT_PTR xstrcpynA(char *pString1, const char *pString2, UINT_PTR dwMaxLength);
INT_PTR xstrcpynW(wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength);
BOOL xstrstrA(const char *pText, INT_PTR nTextLen, const char *pStr, int nStrLen, BOOL bSensitive, char **pStrBegin, char **pStrEnd);
BOOL xstrstrW(const wchar_t *wpText, INT_PTR nTextLen, const wchar_t *wpStr, int nStrLen, BOOL bSensitive, wchar_t **wpStrBegin, wchar_t **wpStrEnd);
int xstrrepA(const char *pText, INT_PTR nTextLen, const char *pIt, int nItLen, const char *pWith, int nWithLen, BOOL bSensitive, char *szResult, INT_PTR *nResultLen);
int xstrrepW(const wchar_t *wpText, INT_PTR nTextLen, const wchar_t *wpIt, int nItLen, const wchar_t *wpWith, int nWithLen, BOOL bSensitive, wchar_t *wszResult, INT_PTR *nResultLen);

INT_PTR xatoiA(const char *pStr, const char **pNext);
INT_PTR xatoiW(const wchar_t *wpStr, const wchar_t **wpNext);
__int64 xatoi64A(const char *pStr, const char **pNext);
__int64 xatoi64W(const wchar_t *wpStr, const wchar_t **wpNext);
int xitoaA(INT_PTR nNumber, char *szStr);
int xitoaW(INT_PTR nNumber, wchar_t *wszStr);
int xuitoaA(UINT_PTR nNumber, char *szStr);
int xuitoaW(UINT_PTR nNumber, wchar_t *wszStr);
int xi64toaA(__int64 nNumber, char *szStr);
int xi64toaW(__int64 nNumber, wchar_t *wszStr);
INT_PTR hex2decA(const char *pStrHex);
INT_PTR hex2decW(const wchar_t *wpStrHex);
int dec2hexA(UINT_PTR nDec, char *szStrHex, unsigned int nWidth, BOOL bLowerCase);
int dec2hexW(UINT_PTR nDec, wchar_t *wszStrHex, unsigned int nWidth, BOOL bLowerCase);
INT_PTR bin2hexA(const unsigned char *pData, INT_PTR nBytes, char *szStrHex, INT_PTR nStrHexMax, BOOL bLowerCase);
INT_PTR bin2hexW(const unsigned char *pData, INT_PTR nBytes, wchar_t *wszStrHex, INT_PTR nStrHexMax, BOOL bLowerCase);
INT_PTR hex2binA(const char *pStrHex, unsigned char *pData, INT_PTR nDataMax);
INT_PTR hex2binW(const wchar_t *wpStrHex, unsigned char *pData, INT_PTR nDataMax);
INT_PTR xprintfA(char *szOutput, const char *pFormat, ...);
INT_PTR xprintfW(wchar_t *wszOutput, const wchar_t *wpFormat, ...);

UINT_PTR UTF16toUTF8(const unsigned short *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned char *szTarget, UINT_PTR nTargetMax);
UINT_PTR UTF8toUTF16(const unsigned char *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned short *szTarget, UINT_PTR nTargetMax);
UINT_PTR UTF32toUTF16(const unsigned long *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned short *szTarget, UINT_PTR nTargetMax);
UINT_PTR UTF16toUTF32(const unsigned short *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned long *szTarget, UINT_PTR nTargetMax);

#endif

//Make sure max and min macros defined
#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


/********************************************************************
 *
 *  WideCharLower
 *
 *Translate wide character to lowercase.
 *
 * [in] wchar_t c  Unicode character.
 *
 *Returns: lowercase unicode character.
 ********************************************************************/
#if defined WideCharLower || defined ALLSTRFUNC
#define WideCharLower_INCLUDED
#undef WideCharLower
wchar_t WideCharLower(wchar_t c)
{
  if (c < 0x100)
  {
    if ((c >= 0x0041 && c <= 0x005a) ||
        (c >= 0x00c0 && c <= 0x00de))
      return (c + 0x20);

    if (c == 0x00b5)
      return 0x03bc;

    return c;
  }
  else if (c < 0x300)
  {
    if ((c >= 0x0100 && c <= 0x012e) ||
        (c >= 0x0132 && c <= 0x0136) ||
        (c >= 0x014a && c <= 0x0176) ||
        (c >= 0x01de && c <= 0x01ee) ||
        (c >= 0x01f8 && c <= 0x021e) ||
        (c >= 0x0222 && c <= 0x0232))
    {
      if (!(c & 0x01))
        return (c + 1);
      return c;
    }

    if ((c >= 0x0139 && c <= 0x0147) ||
        (c >= 0x01cd && c <= 0x91db))
    {
      if (c & 0x01)
        return (c + 1);
      return c;
    }

    if (c >= 0x178 && c <= 0x01f7)
    {
      wchar_t k;

      switch (c)
      {
        case 0x0178:
          k=0x00ff;
          break;
        case 0x0179:
        case 0x017b:
        case 0x017d:
        case 0x0182:
        case 0x0184:
        case 0x0187:
        case 0x018b:
        case 0x0191:
        case 0x0198:
        case 0x01a0:
        case 0x01a2:
        case 0x01a4:
        case 0x01a7:
        case 0x01ac:
        case 0x01af:
        case 0x01b3:
        case 0x01b5:
        case 0x01b8:
        case 0x01bc:
        case 0x01c5:
        case 0x01c8:
        case 0x01cb:
        case 0x01cd:
        case 0x01cf:
        case 0x01d1:
        case 0x01d3:
        case 0x01d5:
        case 0x01d7:
        case 0x01d9:
        case 0x01db:
        case 0x01f2:
        case 0x01f4:
          k=c + 1;
          break;
        case 0x017f:
          k=0x0073;
          break;
        case 0x0181:
          k=0x0253;
          break;
        case 0x0186:
          k=0x0254;
          break;
        case 0x0189:
          k=0x0256;
          break;
        case 0x018a:
          k=0x0257;
          break;
        case 0x018e:
          k=0x01dd;
          break;
        case 0x018f:
          k=0x0259;
          break;
        case 0x0190:
          k=0x025b;
          break;
        case 0x0193:
          k=0x0260;
          break;
        case 0x0194:
          k=0x0263;
          break;
        case 0x0196:
          k=0x0269;
          break;
        case 0x0197:
          k=0x0268;
          break;
        case 0x019c:
          k=0x026f;
          break;
        case 0x019d:
          k=0x0272;
          break;
        case 0x019f:
          k=0x0275;
          break;
        case 0x01a6:
          k=0x0280;
          break;
        case 0x01a9:
          k=0x0283;
          break;
        case 0x01ae:
          k=0x0288;
          break;
        case 0x01b1:
          k=0x028a;
          break;
        case 0x01b2:
          k=0x028b;
          break;
        case 0x01b7:
          k=0x0292;
          break;
        case 0x01c4:
        case 0x01c7:
        case 0x01ca:
        case 0x01f1:
          k=c + 2;
          break;
        case 0x01f6:
          k=0x0195;
          break;
        case 0x01f7:
          k=0x01bf;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
    if (c == 0x0220)
      return 0x019e;
  }
  else if (c < 0x0400)
  {
    if (c >= 0x0391 && c <= 0x03ab && c != 0x03a2)
      return (c + 0x20);
    if (c >= 0x03d8 && c <= 0x03ee && !(c & 0x01))
      return (c + 1);
    if (c >= 0x0386 && c <= 0x03f5)
    {
      wchar_t k;

      switch (c)
      {
        case 0x0386:
          k=0x03ac;
          break;
        case 0x0388:
          k=0x03ad;
          break;
        case 0x0389:
          k=0x03ae;
          break;
        case 0x038a:
          k=0x03af;
          break;
        case 0x038c:
          k=0x03cc;
          break;
        case 0x038e:
          k=0x03cd;
          break;
        case 0x038f:
          k=0x038f;
          break;
        case 0x03c2:
          k=0x03c3;
          break;
        case 0x03d0:
          k=0x03b2;
          break;
        case 0x03d1:
          k=0x03b8;
          break;
        case 0x03d5:
          k=0x03c6;
          break;
        case 0x03d6:
          k=0x03c0;
          break;
        case 0x03f0:
          k=0x03ba;
          break;
        case 0x03f1:
          k=0x03c1;
          break;
        case 0x03f2:
          k=0x03c3;
          break;
        case 0x03f4:
          k=0x03b8;
          break;
        case 0x03f5:
          k=0x03b5;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
    if (c == 0x0345)
      return 0x03b9;
  }
  else if (c < 0x500)
  {
    if (c >= 0x0400 && c <= 0x040f)
      return (c + 0x50);

    if (c >= 0x0410 && c <= 0x042f)
      return (c + 0x20);

    if ((c >= 0x0460 && c <= 0x0480) ||
        (c >= 0x048a && c <= 0x04be) ||
        (c >= 0x04d0 && c <= 0x04f4) ||
        (c == 0x04f8))
    {
      if (!(c & 0x01))
        return (c + 1);
      return c;
    }

    if (c >= 0x04c1 && c <= 0x04cd)
    {
      if (c & 0x01)
        return (c + 1);
      return c;
    }
  }
  else if (c < 0x1f00)
  {
    if ((c >= 0x0500 && c <= 0x050e) ||
        (c >= 0x1e00 && c <= 0x1e94) ||
        (c >= 0x1ea0 && c <= 0x1ef8))
    {
      if (!(c & 0x01))
        return (c + 1);
      return c;
    }

    if (c >= 0x0531 && c <= 0x0556)
      return (c + 0x30);

    if (c == 0x1e9b)
      return 0x1e61;
  }
  else if (c < 0x2000)
  {
    if ((c >= 0x1f08 && c <= 0x1f0f) ||
        (c >= 0x1f18 && c <= 0x1f1d) ||
        (c >= 0x1f28 && c <= 0x1f2f) ||
        (c >= 0x1f38 && c <= 0x1f3f) ||
        (c >= 0x1f48 && c <= 0x1f4d) ||
        (c >= 0x1f68 && c <= 0x1f6f) ||
        (c >= 0x1f88 && c <= 0x1f8f) ||
        (c >= 0x1f98 && c <= 0x1f9f) ||
        (c >= 0x1fa8 && c <= 0x1faf))
      return (c - 0x08);

    if (c >= 0x1f59 && c <= 0x1f5f)
    {
      if (c & 0x01)
        return (c - 0x08);
      return c;
    }

    if (c >= 0x1fb8 && c <= 0x1ffc)
    {
      wchar_t k;

      switch (c)
      {
        case 0x1fb8:
        case 0x1fb9:
        case 0x1fd8:
        case 0x1fd9:
        case 0x1fe8:
        case 0x1fe9:
          k=c - 0x08;
          break;
        case 0x1fba:
        case 0x1fbb:
          k=c - 0x4a;
          break;
        case 0x1fbc:
          k=0x1fb3;
          break;
        case 0x1fbe:
          k=0x03b9;
          break;
        case 0x1fc8:
        case 0x1fc9:
        case 0x1fca:
        case 0x1fcb:
          k=c - 0x56;
          break;
        case 0x1fcc:
          k=0x1fc3;
          break;
        case 0x1fda:
        case 0x1fdb:
          k=c - 0x64;
          break;
        case 0x1fea:
        case 0x1feb:
          k=c - 0x70;
          break;
        case 0x1fec:
          k=0x1fe5;
          break;
        case 0x1ffa:
        case 0x1ffb:
          k=c - 0x7e;
          break;
        case 0x1ffc:
          k=0x1ff3;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
  }
  else
  {
    if (c >= 0x2160 && c <= 0x216f)
      return (c + 0x10);

    if (c >= 0x24b6 && c <= 0x24cf)
      return (c + 0x1a);

    if (c >= 0xff21 && c <= 0xff3a)
      return (c + 0x20);

    //if (c >= 0x10400 && c <= 0x10425)
    //  return (c + 0x28);

    if (c == 0x2126)
      return 0x03c9;
    if (c == 0x212a)
      return 0x006b;
    if (c == 0x212b)
      return 0x00e5;
  }
  return c;
}
#endif

/********************************************************************
 *
 *  WideCharUpper
 *
 *Translate wide character to uppercase.
 *
 * [in] wchar_t c  Unicode character.
 *
 *Returns: uppercase unicode character.
 ********************************************************************/
#if defined WideCharUpper || defined ALLSTRFUNC
#define WideCharUpper_INCLUDED
#undef WideCharUpper
wchar_t WideCharUpper(wchar_t c)
{
  if (c < 0x100)
  {
    if ((c >= 0x0061 && c <= 0x007a) ||
        (c >= 0x00e0 && c <= 0x00fe))
      return (c - 0x20);

    if (c == 0x00b5)
      return 0x039c;

    if (c == 0xff)
      return 0x0178;

    return c;
  }
  else if (c < 0x300)
  {
    if ((c >= 0x0101 && c <= 0x012f) ||
        (c >= 0x0133 && c <= 0x0137) ||
        (c >= 0x014b && c <= 0x0177) ||
        (c >= 0x01df && c <= 0x01ef) ||
        (c >= 0x01f9 && c <= 0x021f) ||
        (c >= 0x0223 && c <= 0x0233))
    {
      if (c & 0x01)
        return (c - 1);
      return c;
    }

    if ((c >= 0x013a && c <= 0x0148) ||
        (c >= 0x01ce && c <= 0x1dc))
    {
      if (!(c & 0x01))
        return (c - 1);
      return c;
    }

    if (c == 0x0131)
      return 0x0049;

    if (c == 0x017a || c == 0x017c || c == 0x017e)
      return (c - 1);

    if (c >= 0x017f && c <= 0x0292)
    {
      wchar_t k;

      switch (c)
      {
        case 0x017f:
          k=0x0053;
          break;
        case 0x0183:
          k=0x0182;
          break;
        case 0x0185:
          k=0x0184;
          break;
        case 0x0188:
          k=0x0187;
          break;
        case 0x018c:
          k=0x018b;
          break;
        case 0x0192:
          k=0x0191;
          break;
        case 0x0195:
          k=0x01f6;
          break;
        case 0x0199:
          k=0x0198;
          break;
        case 0x019e:
          k=0x0220;
          break;
        case 0x01a1:
        case 0x01a3:
        case 0x01a5:
        case 0x01a8:
        case 0x01ad:
        case 0x01b0:
        case 0x01b4:
        case 0x01b6:
        case 0x01b9:
        case 0x01bd:
        case 0x01c5:
        case 0x01c8:
        case 0x01cb:
        case 0x01f2:
        case 0x01f5:
          k=c - 1;
          break;
        case 0x01bf:
          k=0x01f7;
          break;
        case 0x01c6:
        case 0x01c9:
        case 0x01cc:
          k=c - 2;
          break;
        case 0x01dd:
          k=0x018e;
          break;
        case 0x01f3:
          k=0x01f1;
          break;
        case 0x0253:
          k=0x0181;
          break;
        case 0x0254:
          k=0x0186;
          break;
        case 0x0256:
          k=0x0189;
          break;
        case 0x0257:
          k=0x018a;
          break;
        case 0x0259:
          k=0x018f;
          break;
        case 0x025b:
          k=0x0190;
          break;
        case 0x0260:
          k=0x0193;
          break;
        case 0x0263:
          k=0x0194;
          break;
        case 0x0268:
          k=0x0197;
          break;
        case 0x0269:
          k=0x0196;
          break;
        case 0x026f:
          k=0x019c;
          break;
        case 0x0272:
          k=0x019d;
          break;
        case 0x0275:
          k=0x019f;
          break;
        case 0x0280:
          k=0x01a6;
          break;
        case 0x0283:
          k=0x01a9;
          break;
        case 0x0288:
          k=0x01ae;
          break;
        case 0x028a:
          k=0x01b1;
          break;
        case 0x028b:
          k=0x01b2;
          break;
        case 0x0292:
          k=0x01b7;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
  }
  else if (c < 0x0400)
  {
    if (c == 0x03ac)
      return 0x0386;

    if ((c & 0xfff0) == 0x03a0 && c >= 0x03ad)
      return (c - 0x15);

    if (c >= 0x03b1 && c <= 0x03cb && c != 0x03c2)
      return (c - 0x20);

    if (c == 0x03c2)
      return 0x03a3;

    if (c >= 0x03cc && c <= 0x03f5)
    {
      wchar_t k;

      switch (c)
      {
        case 0x03cc:
          k=0x038c;
          break;
        case 0x03cd:
        case 0x03ce:
          k=c - 0x3f;
          break;
        case 0x03d0:
          k=0x0392;
          break;
        case 0x03d1:
          k=0x0398;
          break;
        case 0x03d5:
          k=0x03a6;
          break;
        case 0x03d6:
          k=0x03a0;
          break;
        case 0x03d9:
        case 0x03db:
        case 0x03dd:
        case 0x03df:
        case 0x03e1:
        case 0x03e3:
        case 0x03e5:
        case 0x03e7:
        case 0x03e9:
        case 0x03eb:
        case 0x03ed:
        case 0x03ef:
          k=c - 1;
          break;
        case 0x03f0:
          k=0x039a;
          break;
        case 0x03f1:
          k=0x03a1;
          break;
        case 0x03f2:
          k=0x03a3;
          break;
        case 0x03f5:
          k=0x0395;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
  }
  else if (c < 0x500)
  {
    if (c >= 0x0450 && c <= 0x045f)
      return (c - 0x50);

    if (c >= 0x0430 && c <= 0x044f)
      return (c - 0x20);

    if ((c >= 0x0461 && c <= 0x0481) ||
        (c >= 0x048b && c <= 0x04bf) ||
        (c >= 0x04d1 && c <= 0x04f5))
    {
      if (c & 0x01)
        return (c - 1);
      return c;
    }

    if (c >= 0x04c2 && c <= 0x04ce)
    {
      if (!(c & 0x01))
        return (c - 1);
      return c;
    }

    if (c == 0x04f9)
      return 0x04f8;
  }
  else if (c < 0x1f00)
  {
    if ((c >= 0x0501 && c <= 0x050f) ||
        (c >= 0x1e01 && c <= 0x1e95) ||
        (c >= 0x1ea1 && c <= 0x1ef9))
    {
      if (c & 0x01)
        return (c - 1);
      return c;
    }

    if (c >= 0x0561 && c <= 0x0586)
      return (c - 0x30);

    if (c == 0x1e9b)
      return 0x1e60;
  }
  else if (c < 0x2000)
  {
    if ((c >= 0x1f00 && c <= 0x1f07) ||
        (c >= 0x1f10 && c <= 0x1f15) ||
        (c >= 0x1f20 && c <= 0x1f27) ||
        (c >= 0x1f30 && c <= 0x1f37) ||
        (c >= 0x1f40 && c <= 0x1f45) ||
        (c >= 0x1f60 && c <= 0x1f67) ||
        (c >= 0x1f80 && c <= 0x1f87) ||
        (c >= 0x1f90 && c <= 0x1f97) ||
        (c >= 0x1fa0 && c <= 0x1fa7))
      return (c + 0x08);

    if (c >= 0x1f51 && c <= 0x1f57 && (c & 0x01))
      return (c + 0x08);

    if (c >= 0x1f70 && c <= 0x1ff3)
    {
      wchar_t k;

      switch (c)
      {
        case 0x1fb0:
          k=0x1fb8;
          break;
        case 0x1fb1:
          k=0x1fb9;
          break;
        case 0x1f70:
          k=0x1fba;
          break;
        case 0x1f71:
          k=0x1fbb;
          break;
        case 0x1fb3:
          k=0x1fbc;
          break;
        case 0x1fbe:
          k=0x0399;
          break;
        case 0x1f72:
          k=0x1fc8;
          break;
        case 0x1f73:
          k=0x1fc9;
          break;
        case 0x1f74:
          k=0x1fca;
          break;
        case 0x1f75:
          k=0x1fcb;
          break;
        case 0x1fd0:
          k=0x1fd8;
          break;
        case 0x1fd1:
          k=0x1fd9;
          break;
        case 0x1f76:
          k=0x1fda;
          break;
        case 0x1f77:
          k=0x1fdb;
          break;
        case 0x1fe0:
          k=0x1fe8;
          break;
        case 0x1fe1:
          k=0x1fe9;
          break;
        case 0x1f7a:
          k=0x1fea;
          break;
        case 0x1f7b:
          k=0x1feb;
          break;
        case 0x1fe5:
          k=0x1fec;
          break;
        case 0x1f78:
          k=0x1ff8;
          break;
        case 0x1f79:
          k=0x1ff9;
          break;
        case 0x1f7c:
          k=0x1ffa;
          break;
        case 0x1f7d:
          k=0x1ffb;
          break;
        case 0x1ff3:
          k=0x1ffc;
          break;
        default:
          k=0;
      }
      if (k != 0)
        return k;
    }
  }
  else
  {
    if (c >= 0x2170 && c <= 0x217f)
      return (c - 0x10);

    if (c >= 0x24d0 && c <= 0x24e9)
      return (c - 0x1a);

    if (c >= 0xff41 && c <= 0xff5a)
      return (c - 0x20);

//    if (c >= 0x10428 && c <= 0x1044d)
//      return (c - 0x28);
  }
  return c;
}
#endif

/********************************************************************
 *
 *  xmemcpy
 *
 *Copies characters between buffers.
 *
 *[out] void *dest       New buffer.
 * [in] const void *src  Buffer to copy from.
 * [in] UINT_PTR count   Number of bytes to copy.
 *
 *Returns:  the value of dest.
 ********************************************************************/
#if defined xmemcpy || defined ALLSTRFUNC
#define xmemcpy_INCLUDED
#undef xmemcpy
void* xmemcpy(void *dest, const void *src, UINT_PTR count)
{
  unsigned char *byte_dest=(unsigned char *)dest;
  unsigned char *byte_src=(unsigned char *)src;

  //Special form of memcpy implementation to avoid
  //compiler from replace this code with memcpy call.
  if (byte_dest != byte_src)
  {
    if (count)
    {
      for (;;)
      {
        *byte_dest=*byte_src;
        if (!--count) break;
        ++byte_dest;
        ++byte_src;
      }
    }
  }
  return dest;
}
#endif

/********************************************************************
 *
 *  xmemcmp
 *
 *Compare characters in two buffers.
 *
 *[in] const void *buf1  First buffer.
 *[in] const void *buf2  Second buffer.
 *[in] UINT_PTR count    Number of bytes.
 *
 *Returns:  -1 buf1 less than buf2.
 *           0 buf1 identical to buf2.
 *           1 buf1 greater than buf2.
 ********************************************************************/
#if defined xmemcmp || defined ALLSTRFUNC
#define xmemcmp_INCLUDED
#undef xmemcmp
int xmemcmp(const void *buf1, const void *buf2, UINT_PTR count)
{
  unsigned char *byte_buf1=(unsigned char *)buf1;
  unsigned char *byte_buf2=(unsigned char *)buf2;

  if (byte_buf1 != byte_buf2)
  {
    while (count--)
    {
      if (*byte_buf1 == *byte_buf2)
      {
        ++byte_buf1;
        ++byte_buf2;
        continue;
      }
      else if (*byte_buf1 < *byte_buf2)
        return -1;
      else
        return 1;
    }
  }
  return 0;
}
#endif

/********************************************************************
 *
 *  xmemset
 *
 *Compare characters in two buffers.
 *
 *[out] void *dest      Pointer to destination.
 * [in] int c           Character to set.
 * [in] UINT_PTR count  Number of characters.
 *
 *Returns:  the value of dest.
 ********************************************************************/
#if defined xmemset || defined ALLSTRFUNC
#define xmemset_INCLUDED
#undef xmemset
void* xmemset(void *dest, int c, UINT_PTR count)
{
  unsigned char *byte_dest=(unsigned char *)dest;

  //Special form of memset implementation to avoid
  //compiler from replace this code with memset call.
  if (count)
  {
    for (;;)
    {
      *byte_dest=(unsigned char)c;
      if (!--count) break;
      ++byte_dest;
    }
  }
  return dest;
}
#endif

/********************************************************************
 *
 *  xarraysizeA
 *
 *Retrieves size of the string that is terminated with two NULL characters.
 *
 *[in]  const char *pString  Pointer to a string terminated with two NULL characters.
 *[out] int *nElements       Number of elements in array, each element separated by NULL character. Can be NULL.
 *
 *Returns:  number of characters, including two NULL characters.
 ********************************************************************/
#if defined xarraysizeA || defined ALLSTRFUNC
#define xarraysizeA_INCLUDED
#undef xarraysizeA
INT_PTR xarraysizeA(const char *pString, int *nElements)
{
  const char *pCount=pString;
  int nCount=1;

  if (!pCount) return 0;

  for (;;)
  {
    if (*pCount == '\0')
    {
      if (*++pCount == '\0')
        break;
      ++nCount;
    }
    ++pCount;
  }
  if (nElements) *nElements=nCount;
  return (pCount - pString) + 1;
}
#endif

/********************************************************************
 *
 *  xarraysizeW
 *
 *Retrieves size of the string that is terminated with two NULL characters.
 *
 *[in]  const wchar_t *wpString  Pointer to a string terminated with two NULL characters.
 *[out] int *nElements           Number of elements in array, each element separated by NULL character. Can be NULL.
 *
 *Returns:  number of characters, including two NULL characters.
 ********************************************************************/
#if defined xarraysizeW || defined ALLSTRFUNC
#define xarraysizeW_INCLUDED
#undef xarraysizeW
INT_PTR xarraysizeW(const wchar_t *wpString, int *nElements)
{
  const wchar_t *wpCount=wpString;
  int nCount=1;

  if (!wpCount) return 0;

  for (;;)
  {
    if (*wpCount == L'\0')
    {
      if (*++wpCount == L'\0')
        break;
      ++nCount;
    }
    ++wpCount;
  }
  if (nElements) *nElements=nCount;
  return (wpCount - wpString) + 1;
}
#endif

/********************************************************************
 *
 *  xstrlenA
 *
 *Retrieves length of the string that is terminated with NULL character.
 *
 *[in] const char *pString   Pointer to a string terminated with NULL character.
 *
 *Returns:  number of characters, not including the terminating NULL character.
 ********************************************************************/
#if defined xstrlenA || defined ALLSTRFUNC
#define xstrlenA_INCLUDED
#undef xstrlenA
INT_PTR xstrlenA(const char *pString)
{
  const char *pCount;

  if (!pString) return 0;
  for (pCount=pString; *pCount != '\0'; ++pCount);
  return (pCount - pString);
}
#endif

/********************************************************************
 *
 *  xstrlenW
 *
 *Retrieves length of the string that is terminated with NULL character.
 *
 *[in] const wchar_t *wpString   Pointer to a string terminated with NULL character.
 *
 *Returns:  number of characters, not including the terminating NULL character.
 ********************************************************************/
#if defined xstrlenW || defined ALLSTRFUNC
#define xstrlenW_INCLUDED
#undef xstrlenW
INT_PTR xstrlenW(const wchar_t *wpString)
{
  const wchar_t *wpCount;

  if (!wpString) return 0;
  for (wpCount=wpString; *wpCount != L'\0'; ++wpCount);
  return (wpCount - wpString);
}
#endif

/********************************************************************
 *
 *  xstrcmpA
 *
 *Case sensitive comparison of two strings.
 *
 *[in] char *pString1  First string to compare.
 *[in] char *pString2  Second string to compare.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpA || defined ALLSTRFUNC
#define xstrcmpA_INCLUDED
#undef xstrcmpA
int xstrcmpA(const char *pString1, const char *pString2)
{
  while (*pString1)
  {
    if (*pString1 != *pString2)
      return ((WORD)*pString1 > (WORD)*pString2)?1:-1;

    ++pString1;
    ++pString2;
  }

  if (*pString1 == *pString2)
    return 0;
  return -1;
}
#endif

/********************************************************************
 *
 *  xstrcmpW
 *
 *Case sensitive comparison of two unicode strings.
 *
 *[in] wchar_t *wpString1  First string to compare.
 *[in] wchar_t *wpString2  Second string to compare.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpW || defined ALLSTRFUNC
#define xstrcmpW_INCLUDED
#undef xstrcmpW
int xstrcmpW(const wchar_t *wpString1, const wchar_t *wpString2)
{
  while (*wpString1)
  {
    if (*wpString1 != *wpString2)
      return ((WORD)*wpString1 > (WORD)*wpString2)?1:-1;

    ++wpString1;
    ++wpString2;
  }

  if (*wpString1 == *wpString2)
    return 0;
  return -1;
}
#endif

/********************************************************************
 *
 *  xstrcmpiA
 *
 *Case insensitive comparison of two strings.
 *
 *[in] char *pString1  First string to compare.
 *[in] char *pString2  Second string to compare.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpiA || defined ALLSTRFUNC
#define xstrcmpiA_INCLUDED
#undef xstrcmpiA
int xstrcmpiA(const char *pString1, const char *pString2)
{
  INT_PTR nCompare;

  while (*pString1)
  {
    if (nCompare=(INT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pString1) - (INT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pString2))
      return (nCompare > 0)?1:-1;

    ++pString1;
    ++pString2;
  }

  if (*pString1 == *pString2)
    return 0;
  return -1;
}
#endif

/********************************************************************
 *
 *  xstrcmpiW
 *
 *Case insensitive comparison of two unicode strings.
 *
 *[in] wchar_t *wpString1  First string to compare.
 *[in] wchar_t *wpString2  Second string to compare.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 *
 *Note:
 *  xstrcmpiW can be used on Win95/98/Me if WideCharLower or WideCharUpper defined.
 ********************************************************************/
#if defined xstrcmpiW || defined ALLSTRFUNC
#define xstrcmpiW_INCLUDED
#undef xstrcmpiW
int xstrcmpiW(const wchar_t *wpString1, const wchar_t *wpString2)
{
  INT_PTR nCompare;

  while (*wpString1)
  {
    #if defined WideCharLower_INCLUDED
      if (nCompare=WideCharLower(*wpString1) - WideCharLower(*wpString2))
        return (nCompare > 0)?1:-1;
    #elif defined WideCharUpper_INCLUDED
      if (nCompare=WideCharUpper(*wpString1) - WideCharUpper(*wpString2))
        return (nCompare > 0)?1:-1;
    #else
      #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrcmpiW will not work on Win95/98/Me.")
      if (nCompare=(INT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpString1) - (INT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpString2))
        return (nCompare > 0)?1:-1;
    #endif

    ++wpString1;
    ++wpString2;
  }

  if (*wpString1 == *wpString2)
    return 0;
  return -1;
}
#endif

/********************************************************************
 *
 *  xstrcmpnA
 *
 *Case sensitive comparison specified number of characters of two strings.
 *
 *[in] char *pString1     First string to compare.
 *[in] char *pString2     Second string to compare.
 *[in] DWORD dwMaxLength  Number of characters to compare,
 *                         -1 compare until NULL character in pString1.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpnA || defined ALLSTRFUNC
#define xstrcmpnA_INCLUDED
#undef xstrcmpnA
int xstrcmpnA(const char *pString1, const char *pString2, UINT_PTR dwMaxLength)
{
  UINT_PTR dwCount=dwMaxLength;

  while (dwCount && *pString1)
  {
    if (*pString1 != *pString2)
      return ((WORD)*pString1 > (WORD)*pString2)?1:-1;
    ++pString1;
    ++pString2;
    --dwCount;
  }
  if (!dwCount) return 0;
  if (!*pString2) return 0;
  return (dwMaxLength == (UINT_PTR)-1)?0:-1;
}
#endif

/********************************************************************
 *
 *  xstrcmpnW
 *
 *Case sensitive comparison specified number of characters of two unicode strings.
 *
 *[in] wchar_t *wpString1  First string to compare.
 *[in] wchar_t *wpString2  Second string to compare.
 *[in] DWORD dwMaxLength   Number of characters to compare,
 *                          -1 compare until NULL character in wpString1.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpnW || defined ALLSTRFUNC
#define xstrcmpnW_INCLUDED
#undef xstrcmpnW
int xstrcmpnW(const wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength)
{
  UINT_PTR dwCount=dwMaxLength;

  while (dwCount && *wpString1)
  {
    if (*wpString1 != *wpString2)
      return ((WORD)*wpString1 > (WORD)*wpString2)?1:-1;

    ++wpString1;
    ++wpString2;
    --dwCount;
  }
  if (!dwCount) return 0;
  if (!*wpString2) return 0;
  return (dwMaxLength == (UINT_PTR)-1)?0:-1;
}
#endif

/********************************************************************
 *
 *  xstrcmpinA
 *
 *Case insensitive comparison specified number of characters of two strings.
 *
 *[in] char *pString1     First string to compare.
 *[in] char *pString2     Second string to compare.
 *[in] DWORD dwMaxLength  Number of characters to compare,
 *                         -1 compare until NULL character in pString1.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 ********************************************************************/
#if defined xstrcmpinA || defined ALLSTRFUNC
#define xstrcmpinA_INCLUDED
#undef xstrcmpinA
int xstrcmpinA(const char *pString1, const char *pString2, UINT_PTR dwMaxLength)
{
  UINT_PTR dwCount=dwMaxLength;
  INT_PTR nCompare;

  while (dwCount && *pString1)
  {
    if (nCompare=(INT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pString1) - (INT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pString2))
      return (nCompare > 0)?1:-1;

    ++pString1;
    ++pString2;
    --dwCount;
  }
  if (!dwCount) return 0;
  if (!*pString2) return 0;
  return (dwMaxLength == (UINT_PTR)-1)?0:-1;
}
#endif

/********************************************************************
 *
 *  xstrcmpinW
 *
 *Case insensitive comparison specified number of characters of two unicode strings.
 *
 *[in] wchar_t *wpString1  First string to compare.
 *[in] wchar_t *wpString2  Second string to compare.
 *[in] DWORD dwMaxLength   Number of characters to compare,
 *                          -1 compare until NULL character in wpString1.
 *
 *Returns:  -1 string1 less than string2.
 *           0 string1 identical to string2.
 *           1 string1 greater than string2.
 *
 *Note:
 *  xstrcmpinW can be used on Win95/98/Me if WideCharLower or WideCharUpper defined.
 ********************************************************************/
#if defined xstrcmpinW || defined ALLSTRFUNC
#define xstrcmpinW_INCLUDED
#undef xstrcmpinW
int xstrcmpinW(const wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength)
{
  UINT_PTR dwCount=dwMaxLength;
  INT_PTR nCompare;

  while (dwCount && *wpString1)
  {
    #if defined WideCharLower_INCLUDED
      if (nCompare=WideCharLower(*wpString1) - WideCharLower(*wpString2))
        return (nCompare > 0)?1:-1;
    #elif defined WideCharUpper_INCLUDED
      if (nCompare=WideCharUpper(*wpString1) - WideCharUpper(*wpString2))
        return (nCompare > 0)?1:-1;
    #else
      #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrcmpinW will not work on Win95/98/Me.")
      if (nCompare=(INT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpString1) - (INT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpString2))
        return (nCompare > 0)?1:-1;
    #endif

    ++wpString1;
    ++wpString2;
    --dwCount;
  }
  if (!dwCount) return 0;
  if (!*wpString2) return 0;
  return (dwMaxLength == (UINT_PTR)-1)?0:-1;
}
#endif

/********************************************************************
 *
 *  xstrcpyA
 *
 *Copies a string into a buffer.
 *
 *[in] char *pString1  Pointer to a buffer into which the function copies characters.
 *                      The buffer must be large enough to contain the string,
 *                      including the terminating null character. Can be NULL.
 *[in] char *pString2  Pointer to a null-terminated string from which the function copies characters.
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xstrcpyA uses xstrlenA.
 ********************************************************************/
#if defined xstrcpyA || defined ALLSTRFUNC
#define xstrcpyA_INCLUDED
#undef xstrcpyA
INT_PTR xstrcpyA(char *pString1, const char *pString2)
{
  char *pDest=pString1;
  char *pSrc=(char *)pString2;

  if (!pDest)
    return xstrlenA(pSrc) + 1;
  if (pDest == pSrc)
    return xstrlenA(pSrc);

  while (*pSrc)
    *pDest++=*pSrc++;
  *pDest='\0';
  return pDest - pString1;
}
#endif

/********************************************************************
 *
 *  xstrcpyW
 *
 *Copies a unicode string into a buffer.
 *
 *[in] wchar_t *wpString1  Pointer to a buffer into which the function copies characters.
 *                          The buffer must be large enough to contain the string,
 *                          including the terminating null character. Can be NULL.
 *[in] wchar_t *wpString2  Pointer to a null-terminated string from which the function copies characters.
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xstrcpyW uses xstrlenW.
 ********************************************************************/
#if defined xstrcpyW || defined ALLSTRFUNC
#define xstrcpyW_INCLUDED
#undef xstrcpyW
INT_PTR xstrcpyW(wchar_t *wpString1, const wchar_t *wpString2)
{
  wchar_t *wpDest=wpString1;
  wchar_t *wpSrc=(wchar_t *)wpString2;

  if (!wpDest)
    return xstrlenW(wpSrc) + 1;
  if (wpDest == wpSrc)
    return xstrlenW(wpSrc);

  while (*wpSrc)
    *wpDest++=*wpSrc++;
  *wpDest=L'\0';
  return wpDest - wpString1;
}
#endif

/********************************************************************
 *
 *  xstrcpynA
 *
 *Copies a specified number of characters from a source string into a buffer.
 *
 *[in] char *pString1        Pointer to a buffer into which the function copies characters.
 *                            The buffer must be large enough to contain the number of TCHAR values specified by dwMaxLength,
 *                            including room for a terminating null character. Can be NULL, if dwMaxLength isn't zero.
 *[in] char *pString2        Pointer to a null-terminated string from which the function copies characters.
 *[in] UINT_PTR dwMaxLength  Specifies the number of TCHAR values to be copied from the string pointed to by pString2 into the buffer pointed to by pString1,
 *                            including a terminating null character.
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xstrcpynA uses xstrlenA.
 ********************************************************************/
#if defined xstrcpynA || defined ALLSTRFUNC
#define xstrcpynA_INCLUDED
#undef xstrcpynA
INT_PTR xstrcpynA(char *pString1, const char *pString2, UINT_PTR dwMaxLength)
{
  char *pDest=pString1;
  char *pSrc=(char *)pString2;

  if (!pDest)
    return xstrlenA(pSrc) + 1;
  if (pDest == pSrc)
    return xstrlenA(pSrc);

  while (*pSrc && --dwMaxLength)
    *pDest++=*pSrc++;
  *pDest='\0';
  return pDest - pString1;
}
#endif

/********************************************************************
 *
 *  xstrcpynW
 *
 *Copies a specified number of unicode characters from a source string into a buffer.
 *
 *[in] wchar_t *wpString1   Pointer to a buffer into which the function copies characters.
 *                           The buffer must be large enough to contain the number of TCHAR values specified by dwMaxLength,
 *                           including room for a terminating null character. Can be NULL, if dwMaxLength isn't zero. 
 *[in] wchar_t *wpString2   Pointer to a null-terminated string from which the function copies characters.
 *[in] UINT_PTR dwMaxLength Specifies the number of TCHAR values to be copied from the string pointed to by wpString2 into the buffer pointed to by wpString1,
 *                           including a terminating null character.
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xstrcpynW uses xstrlenW.
 ********************************************************************/
#if defined xstrcpynW || defined ALLSTRFUNC
#define xstrcpynW_INCLUDED
#undef xstrcpynW
INT_PTR xstrcpynW(wchar_t *wpString1, const wchar_t *wpString2, UINT_PTR dwMaxLength)
{
  wchar_t *wpDest=wpString1;
  wchar_t *wpSrc=(wchar_t *)wpString2;

  if (!wpDest)
    return xstrlenW(wpSrc) + 1;
  if (wpDest == wpSrc)
    return xstrlenW(wpSrc);

  while (*wpSrc && --dwMaxLength)
    *wpDest++=*wpSrc++;
  *wpDest=L'\0';
  return wpDest - wpString1;
}
#endif

/********************************************************************
 *
 *  xstrstrA
 *
 *Find substring in string.
 *
 * [in] const char *pText  Text.
 * [in] INT_PTR nTextLen   Text length.
 *                          If this value is -1, the string is assumed to be null-terminated
 *                          and the length is calculated automatically.
 * [in] const char *pStr   Find it.
 * [in] int nStrLen        Find it length.
 *                          If this value is -1, the string is assumed to be null-terminated
 *                          and the length is calculated automatically.
 * [in] BOOL bSensitive    TRUE   case sensitive.
 *                         FALSE  case insensitive.
 *[out] char **pStrBegin   Pointer to the first char of pStr, can be NULL.
 *[out] char **pStrEnd     Pointer to the first char after pStr, can be NULL.
 *
 *Returns:  TRUE  pStr is founded.
 *          FALSE pStr isn't founded.
 *
 *Note:
 *  xstrstrA uses xstrlenA.
 ********************************************************************/
#if defined xstrstrA || defined ALLSTRFUNC
#define xstrstrA_INCLUDED
#undef xstrstrA
BOOL xstrstrA(const char *pText, INT_PTR nTextLen, const char *pStr, int nStrLen, BOOL bSensitive, char **pStrBegin, char **pStrEnd)
{
  const char *pTextMax;
  const char *pTextCount;
  const char *pMatchCount;
  const char *pStrMax;
  const char *pStrCount;

  if (nTextLen == -1)
    nTextLen=xstrlenA(pText) + 1;
  if (nStrLen == -1)
    nStrLen=(int)xstrlenA(pStr);
  pTextMax=pText + nTextLen;
  pStrMax=pStr + nStrLen;

  for (pTextCount=pText; pTextCount < pTextMax; ++pTextCount)
  {
    pMatchCount=pTextCount;
    pStrCount=pStr;

    while (*pMatchCount == *pStrCount ||
           (!bSensitive && (UINT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pMatchCount) == (UINT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pStrCount)))
    {
      if (++pStrCount >= pStrMax)
      {
        if (pStrBegin) *pStrBegin=(char *)pTextCount;
        if (pStrEnd) *pStrEnd=(char *)pMatchCount + 1;
        return TRUE;
      }
      if (++pMatchCount >= pTextMax) break;
    }
  }
  return FALSE;
}
#endif

/********************************************************************
 *
 *  xstrstrW
 *
 *Find substring in unicode string.
 *
 * [in] const wchar_t *wpText  Text.
 * [in] INT_PTR nTextLen       Text length.
 *                              If this value is -1, the string is assumed to be null-terminated
 *                              and the length is calculated automatically.
 * [in] const wchar_t *wpStr   Find it.
 * [in] int nStrLen            Find it length.
 *                              If this value is -1, the string is assumed to be null-terminated
 *                              and the length is calculated automatically.
 * [in] BOOL bSensitive        TRUE   case sensitive.
 *                             FALSE  case insensitive.
 *[out] wchar_t **wpStrBegin   Pointer to the first char of wpStr, can be NULL.
 *[out] wchar_t **wpStrEnd     Pointer to the first char after wpStr, can be NULL.
 *
 *Returns:  TRUE  wpStr is founded.
 *          FALSE wpStr isn't founded.
 *
 *Note:
 *  - xstrstrW uses xstrlenW.
 *  - xstrstrW can be used on Win95/98/Me if WideCharLower or WideCharUpper defined.
 ********************************************************************/
#if defined xstrstrW || defined ALLSTRFUNC
#define xstrstrW_INCLUDED
#undef xstrstrW
BOOL xstrstrW(const wchar_t *wpText, INT_PTR nTextLen, const wchar_t *wpStr, int nStrLen, BOOL bSensitive, wchar_t **wpStrBegin, wchar_t **wpStrEnd)
{
  const wchar_t *wpTextMax;
  const wchar_t *wpTextCount;
  const wchar_t *wpMatchCount;
  const wchar_t *wpStrMax;
  const wchar_t *wpStrCount;

  if (nTextLen == -1)
    nTextLen=xstrlenW(wpText) + 1;
  if (nStrLen == -1)
    nStrLen=(int)xstrlenW(wpStr);
  wpTextMax=wpText + nTextLen;
  wpStrMax=wpStr + nStrLen;

  for (wpTextCount=wpText; wpTextCount < wpTextMax; ++wpTextCount)
  {
    wpMatchCount=wpTextCount;
    wpStrCount=wpStr;

    while (*wpMatchCount == *wpStrCount ||
           #if defined WideCharLower_INCLUDED
             (!bSensitive && WideCharLower(*wpMatchCount) == WideCharLower(*wpStrCount)))
           #elif defined WideCharUpper_INCLUDED
             (!bSensitive && WideCharUpper(*wpMatchCount) == WideCharUpper(*wpStrCount)))
           #else
             #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrstrW will not work on Win95/98/Me.")
             (!bSensitive && (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpMatchCount) == (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpStrCount)))
           #endif
    {
      if (++wpStrCount >= wpStrMax)
      {
        if (wpStrBegin) *wpStrBegin=(wchar_t *)wpTextCount;
        if (wpStrEnd) *wpStrEnd=(wchar_t *)wpMatchCount + 1;
        return TRUE;
      }
      if (++wpMatchCount >= wpTextMax) break;
    }
  }
  return FALSE;
}
#endif

/********************************************************************
 *
 *  xstrrepA
 *
 *Replace substring with string.
 *
 * [in] const char *pText    Text.
 * [in] INT_PTR nTextLen     Text length.
 *                            If this value is -1, the string is assumed to be null-terminated
 *                            and the length is calculated automatically.
 * [in] const char *pIt      Replace it.
 * [in] int nItLen           Replace it length.
 *                            If this value is -1, the string is assumed to be null-terminated
 *                            and the length is calculated automatically.
 * [in] const char *pWith    Replace with.
 * [in] int nWithLen         Replace with length.
 *                            If this value is -1, the string is assumed to be null-terminated
 *                            and the length is calculated automatically.
 * [in] BOOL bSensitive      TRUE   case sensitive.
 *                           FALSE  case insensitive.
 *[out] char *szResult       Output, can be NULL. szResult can be the same pointer as pText, if the nItLen >= nWithLen.
 *[out] INT_PTR *nResultLen  Contains the length of the result string,
 *                            including the terminating null character if nTextLen == -1,
 *                            can be NULL.
 *
 *Returns:  number of changes.
 *
 *Note:
 *  xstrrepA uses xstrlenA.
 ********************************************************************/
#if defined xstrrepA || defined ALLSTRFUNC
#define xstrrepA_INCLUDED
#undef xstrrepA
int xstrrepA(const char *pText, INT_PTR nTextLen, const char *pIt, int nItLen, const char *pWith, int nWithLen, BOOL bSensitive, char *szResult, INT_PTR *nResultLen)
{
  const char *pTextMax;
  const char *pTextCount;
  const char *pMatchCount;
  const char *pItMax;
  const char *pItCount;
  const char *pWithMax;
  const char *pWithCount;
  char *pResultCount;
  int nChanges=0;

  if (nTextLen == -1)
    nTextLen=xstrlenA(pText) + 1;
  if (nItLen == -1)
    nItLen=(int)xstrlenA(pIt);
  if (nWithLen == -1)
    nWithLen=(int)xstrlenA(pWith);
  pTextMax=pText + nTextLen;
  pItMax=pIt + nItLen;
  pWithMax=pWith + nWithLen;
  pResultCount=szResult;

  for (pTextCount=pText; pTextCount < pTextMax; ++pTextCount)
  {
    pMatchCount=pTextCount;
    pItCount=pIt;

    while (*pMatchCount == *pItCount ||
           (!bSensitive && (UINT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pMatchCount) == (UINT_PTR)CharUpperA((char *)(UINT_PTR)(WORD)*pItCount)))
    {
      if (++pItCount >= pItMax)
      {
        if (szResult)
        {
          for (pWithCount=pWith; pWithCount < pWithMax; ++pWithCount)
            *pResultCount++=*pWithCount;
        }
        else pResultCount+=nWithLen;

        pTextCount=pMatchCount + 1;
        pItCount=pIt;
        ++nChanges;
        if (pTextCount >= pTextMax) goto End;
      }
      if (++pMatchCount >= pTextMax) break;
    }
    if (szResult) *pResultCount=*pTextCount;
    ++pResultCount;
  }

  End:
  if (nResultLen) *nResultLen=pResultCount - szResult;
  return nChanges;
}
#endif

/********************************************************************
 *
 *  xstrrepW
 *
 *Replace substring with unicode string.
 *
 * [in] const wchar_t *wpText  Text.
 * [in] INT_PTR nTextLen       Text length.
 *                              If this value is -1, the string is assumed to be null-terminated
 *                              and the length is calculated automatically.
 * [in] const wchar_t *wpIt    Replace it.
 * [in] int nItLen             Replace it length.
 *                              If this value is -1, the string is assumed to be null-terminated
 *                              and the length is calculated automatically.
 * [in] const wchar_t *wpWith  Replace with.
 * [in] int nWithLen           Replace with length.
 *                              If this value is -1, the string is assumed to be null-terminated
 *                              and the length is calculated automatically.
 * [in] BOOL bSensitive        TRUE   case sensitive.
 *                             FALSE  case insensitive.
 *[out] wchar_t *wszResult     Output, can be NULL. wszResult can be the same pointer as wpText, if the nItLen >= nWithLen.
 *[out] INT_PTR *nResultLen    Contains the length of the result string,
 *                              including the terminating null character if nTextLen == -1,
 *                              can be NULL.
 *
 *Returns:  number of changes.
 *
 *Note:
 *  - xstrrepW uses xstrlenW.
 *  - xstrrepW can be used on Win95/98/Me if WideCharLower or WideCharUpper defined.
 ********************************************************************/
#if defined xstrrepW || defined ALLSTRFUNC
#define xstrrepW_INCLUDED
#undef xstrrepW
int xstrrepW(const wchar_t *wpText, INT_PTR nTextLen, const wchar_t *wpIt, int nItLen, const wchar_t *wpWith, int nWithLen, BOOL bSensitive, wchar_t *wszResult, INT_PTR *nResultLen)
{
  const wchar_t *wpTextMax;
  const wchar_t *wpTextCount;
  const wchar_t *wpMatchCount;
  const wchar_t *wpItMax;
  const wchar_t *wpItCount;
  const wchar_t *wpWithMax;
  const wchar_t *wpWithCount;
  wchar_t *wpResultCount;
  int nChanges=0;

  if (nTextLen == -1)
    nTextLen=xstrlenW(wpText) + 1;
  if (nItLen == -1)
    nItLen=(int)xstrlenW(wpIt);
  if (nWithLen == -1)
    nWithLen=(int)xstrlenW(wpWith);
  wpTextMax=wpText + nTextLen;
  wpItMax=wpIt + nItLen;
  wpWithMax=wpWith + nWithLen;
  wpResultCount=wszResult;

  for (wpTextCount=wpText; wpTextCount < wpTextMax; ++wpTextCount)
  {
    wpMatchCount=wpTextCount;
    wpItCount=wpIt;

    while (*wpMatchCount == *wpItCount ||
           #if defined WideCharLower_INCLUDED
             (!bSensitive && WideCharLower(*wpMatchCount) == WideCharLower(*wpItCount)))
           #elif defined WideCharUpper_INCLUDED
             (!bSensitive && WideCharUpper(*wpMatchCount) == WideCharUpper(*wpItCount)))
           #else
             #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrrepW will not work on Win95/98/Me.")
             (!bSensitive && (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpMatchCount) == (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpItCount)))
           #endif
    {
      if (++wpItCount >= wpItMax)
      {
        if (wszResult)
        {
          for (wpWithCount=wpWith; wpWithCount < wpWithMax; ++wpWithCount)
            *wpResultCount++=*wpWithCount;
        }
        else wpResultCount+=nWithLen;

        wpTextCount=wpMatchCount + 1;
        wpItCount=wpIt;
        ++nChanges;
        if (wpTextCount >= wpTextMax) goto End;
      }
      if (++wpMatchCount >= wpTextMax) break;
    }
    if (wszResult) *wpResultCount=*wpTextCount;
    ++wpResultCount;
  }

  End:
  if (nResultLen) *nResultLen=wpResultCount - wszResult;
  return nChanges;
}
#endif

/********************************************************************
 *
 *  xatoiA
 *
 *Converts string to integer.
 *
 *[in]  const char *pStr  String number.
 *[out] char **pNext      Pointer to the first char after number, can be NULL.
 *
 *Returns: integer.
 *
 *Examples:
 *  xatoiA("45", NULL) == 45;
 *  xatoiA("  -0045:value", &pNext) == -45, pNext == ":value"
 ********************************************************************/
#if defined xatoiA || defined ALLSTRFUNC
#define xatoiA_INCLUDED
#undef xatoiA
INT_PTR xatoiA(const char *pStr, const char **pNext)
{
  INT_PTR nNumber=0;
  BOOL bMinus=FALSE;

  while (*pStr == ' ' || *pStr == '\t')
    ++pStr;
  if (*pStr == '+')
    ++pStr;
  else if (*pStr == '-')
  {
    bMinus=TRUE;
    ++pStr;
  }

  while (*pStr >= '0' && *pStr <= '9')
  {
    nNumber=(nNumber * 10) + (*pStr - '0');
    ++pStr;
  }
  if (bMinus == TRUE) nNumber=0 - nNumber;
  if (pNext) *pNext=pStr;
  return nNumber;
}
#endif

/********************************************************************
 *
 *  xatoiW
 *
 *Converts unicode string to integer.
 *
 *[in]  const wchar_t *wpStr  Unicode string number.
 *[out] wchar_t **wpNext      Pointer to the first char after number, can be NULL.
 *
 *Returns: integer.
 *
 *Examples:
 *  xatoiW(L"45", NULL) == 45;
 *  xatoiW(L"  -0045:value", &wpNext) == -45, wpNext == L":value"
 ********************************************************************/
#if defined xatoiW || defined ALLSTRFUNC
#define xatoiW_INCLUDED
#undef xatoiW
INT_PTR xatoiW(const wchar_t *wpStr, const wchar_t **wpNext)
{
  INT_PTR nNumber=0;
  BOOL bMinus=FALSE;

  while (*wpStr == ' ' || *wpStr == '\t')
    ++wpStr;
  if (*wpStr == '+')
    ++wpStr;
  else if (*wpStr == '-')
  {
    bMinus=TRUE;
    ++wpStr;
  }

  while (*wpStr >= '0' && *wpStr <= '9')
  {
    nNumber=(nNumber * 10) + (*wpStr - '0');
    ++wpStr;
  }
  if (bMinus == TRUE) nNumber=0 - nNumber;
  if (wpNext) *wpNext=wpStr;
  return nNumber;
}
#endif

/********************************************************************
 *
 *  xatoi64A
 *
 *Converts string to int64.
 *
 *[in]  const char *pStr  String number.
 *[out] char **pNext      Pointer to the first char after number, can be NULL.
 *
 *Returns: 64-bit integer.
 *
 *Examples:
 *  xatoi64A("45", NULL) == 45;
 *  xatoi64A("  -0045:value", &pNext) == -45, pNext == ":value"
 ********************************************************************/
#if defined xatoi64A || defined ALLSTRFUNC
#define xatoi64A_INCLUDED
#undef xatoi64A
__int64 xatoi64A(const char *pStr, const char **pNext)
{
  __int64 nNumber=0;
  BOOL bMinus=FALSE;

  while (*pStr == ' ' || *pStr == '\t')
    ++pStr;
  if (*pStr == '+')
    ++pStr;
  else if (*pStr == '-')
  {
    bMinus=TRUE;
    ++pStr;
  }

  while (*pStr >= '0' && *pStr <= '9')
  {
    nNumber=(nNumber * 10) + (*pStr - '0');
    ++pStr;
  }
  if (bMinus == TRUE) nNumber=0 - nNumber;
  if (pNext) *pNext=pStr;
  return nNumber;
}
#endif

/********************************************************************
 *
 *  xatoi64W
 *
 *Converts unicode string to int64.
 *
 *[in]  const wchar_t *wpStr  Unicode string number.
 *[out] wchar_t **wpNext      Pointer to the first char after number, can be NULL.
 *
 *Returns: 64-bit integer.
 *
 *Examples:
 *  xatoi64W(L"45", NULL) == 45;
 *  xatoi64W(L"  -0045:value", &wpNext) == -45, wpNext == L":value"
 ********************************************************************/
#if defined xatoi64W || defined ALLSTRFUNC
#define xatoi64W_INCLUDED
#undef xatoi64W
__int64 xatoi64W(const wchar_t *wpStr, const wchar_t **wpNext)
{
  __int64 nNumber=0;
  BOOL bMinus=FALSE;

  while (*wpStr == ' ' || *wpStr == '\t')
    ++wpStr;
  if (*wpStr == '+')
    ++wpStr;
  else if (*wpStr == '-')
  {
    bMinus=TRUE;
    ++wpStr;
  }

  while (*wpStr >= '0' && *wpStr <= '9')
  {
    nNumber=(nNumber * 10) + (*wpStr - '0');
    ++wpStr;
  }
  if (bMinus == TRUE) nNumber=0 - nNumber;
  if (wpNext) *wpNext=wpStr;
  return nNumber;
}
#endif

/********************************************************************
 *
 *  xitoaA
 *
 *Converts integer to string.
 *
 *[in]   INT_PTR nNumber  Integer.
 *[out]  char *szStr      String number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xitoaA(45, szResult);   //szResult == "45"
 *  xitoaA(-45, szResult);  //szResult == "-45"
 ********************************************************************/
#if defined xitoaA || defined ALLSTRFUNC
#define xitoaA_INCLUDED
#undef xitoaA
int xitoaA(INT_PTR nNumber, char *szStr)
{
  char szReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (szStr) szStr[b]='0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (szStr) szStr[b]='-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    szReverse[a]=(char)(nNumber % 10) + '0';
    nNumber=nNumber / 10;
  }
  if (!szStr) return a + b + 1;

  while (--a >= 0) szStr[b++]=szReverse[a];
  szStr[b]='\0';
  return b;
}
#endif

/********************************************************************
 *
 *  xitoaW
 *
 *Converts integer to unicode string.
 *
 *[in]   INT_PTR nNumber  Integer.
 *[out]  wchar_t *wszStr  Unicode string number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xitoaW(45, wszResult);   //wszResult == L"45"
 *  xitoaW(-45, wszResult);  //wszResult == L"-45"
 ********************************************************************/
#if defined xitoaW || defined ALLSTRFUNC
#define xitoaW_INCLUDED
#undef xitoaW
int xitoaW(INT_PTR nNumber, wchar_t *wszStr)
{
  wchar_t wszReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (wszStr) wszStr[b]=L'0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (wszStr) wszStr[b]=L'-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    wszReverse[a]=(wchar_t)(nNumber % 10) + L'0';
    nNumber=nNumber / 10;
  }
  if (!wszStr) return a + b + 1;

  while (--a >= 0) wszStr[b++]=wszReverse[a];
  wszStr[b]=L'\0';
  return b;
}
#endif

/********************************************************************
 *
 *  xuitoaA
 *
 *Converts unsigned integer to string.
 *
 *[in]   UINT_PTR nNumber  Unsigned integer.
 *[out]  char *szStr       String number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xuitoaA(45, szResult);            //szResult == "45"
 *  xuitoaA((UINT_PTR)-1, szResult);  //szResult == "4294967295"
 ********************************************************************/
#if defined xuitoaA || defined ALLSTRFUNC
#define xuitoaA_INCLUDED
#undef xuitoaA
int xuitoaA(UINT_PTR nNumber, char *szStr)
{
  char szReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (szStr) szStr[b]='0';
    ++b;
  }
  for (a=0; nNumber != 0; ++a)
  {
    szReverse[a]=(char)(nNumber % 10) + '0';
    nNumber=nNumber / 10;
  }
  if (!szStr) return a + b + 1;

  while (--a >= 0) szStr[b++]=szReverse[a];
  szStr[b]='\0';
  return b;
}
#endif

/********************************************************************
 *
 *  xuitoaW
 *
 *Converts unsigned integer to unicode string.
 *
 *[in]   UINT_PTR nNumber  Unsigned integer.
 *[out]  wchar_t *wszStr   Unicode string number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xuitoaW(45, wszResult);            //wszResult == L"45"
 *  xuitoaW((UINT_PTR)-1, wszResult);  //wszResult == L"4294967295"
 ********************************************************************/
#if defined xuitoaW || defined ALLSTRFUNC
#define xuitoaW_INCLUDED
#undef xuitoaW
int xuitoaW(UINT_PTR nNumber, wchar_t *wszStr)
{
  wchar_t wszReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (wszStr) wszStr[b]=L'0';
    ++b;
  }
  for (a=0; nNumber != 0; ++a)
  {
    wszReverse[a]=(wchar_t)(nNumber % 10) + L'0';
    nNumber=nNumber / 10;
  }
  if (!wszStr) return a + b + 1;

  while (--a >= 0) wszStr[b++]=wszReverse[a];
  wszStr[b]=L'\0';
  return b;
}
#endif

/********************************************************************
 *
 *  xitoa64A
 *
 *Converts int64 to string.
 *
 *[in]   __int64 nNumber  64-bit integer.
 *[out]  char *szStr      String number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xitoa64A(45, szResult);   //szResult == "45"
 *  xitoa64A(-45, szResult);  //szResult == "-45"
 ********************************************************************/
#if defined xi64toaA || defined ALLSTRFUNC
#define xi64toaA_INCLUDED
#undef xi64toaA
int xi64toaA(__int64 nNumber, char *szStr)
{
  char szReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (szStr) szStr[b]='0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (szStr) szStr[b]='-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    szReverse[a]=(char)(nNumber % 10) + '0';
    nNumber=nNumber / 10;
  }
  if (!szStr) return a + b + 1;

  while (--a >= 0) szStr[b++]=szReverse[a];
  szStr[b]='\0';
  return b;
}
#endif

/********************************************************************
 *
 *  xitoa64W
 *
 *Converts int64 to unicode string.
 *
 *[in]   __int64 nNumber  64-bit integer.
 *[out]  wchar_t *wszStr  Unicode string number, if NULL required buffer size returned in TCHARs.
 *
 *Returns: copied digits.
 *
 *Examples:
 *  xitoa64W(45, wszResult);   //wszResult == L"45"
 *  xitoa64W(-45, wszResult);  //wszResult == L"-45"
 ********************************************************************/
#if defined xi64toaW || defined ALLSTRFUNC
#define xi64toaW_INCLUDED
#undef xi64toaW
int xi64toaW(__int64 nNumber, wchar_t *wszStr)
{
  wchar_t wszReverse[128];
  int a;
  int b=0;

  if (nNumber == 0)
  {
    if (wszStr) wszStr[b]=L'0';
    ++b;
  }
  else if (nNumber < 0)
  {
    if (wszStr) wszStr[b]=L'-';
    ++b;
    nNumber=0 - nNumber;
  }
  for (a=0; nNumber != 0; ++a)
  {
    wszReverse[a]=(wchar_t)(nNumber % 10) + L'0';
    nNumber=nNumber / 10;
  }
  if (!wszStr) return a + b + 1;

  while (--a >= 0) wszStr[b++]=wszReverse[a];
  wszStr[b]=L'\0';
  return b;
}
#endif

/********************************************************************
 *
 *  hex2decA
 *
 *Converts hex value to decimal.
 *
 *[in]  const char *pStrHex  Hex value.
 *
 *Returns: integer. Wrong hex value if equal to -1.
 *
 *Examples:
 *  hex2decA("A1F") == 2591;
 ********************************************************************/
#if defined hex2decA || defined ALLSTRFUNC
#define hex2decA_INCLUDED
#undef hex2decA
INT_PTR hex2decA(const char *pStrHex)
{
  INT_PTR a;
  INT_PTR b=0;

  for (;;)
  {
    a=*pStrHex++;
    if (a >= '0' && a <= '9') a-='0';
    else if (a >= 'a' && a <= 'f') a-='a' - 10;
    else if (a >= 'A' && a <= 'F') a-='A' - 10;
    else return -1;

    if (*pStrHex) b=(b + a) * 16;
    else return (b + a);
  }
}
#endif

/********************************************************************
 *
 *  hex2decW
 *
 *Converts unicode hex value to decimal.
 *
 *[in]  const wchar_t *wpStrHex  Unicode hex value.
 *
 *Returns: integer. Wrong hex value if equal to -1.
 *
 *Examples:
 *  hex2decW(L"A1F") == 2591;
 ********************************************************************/
#if defined hex2decW || defined ALLSTRFUNC
#define hex2decW_INCLUDED
#undef hex2decW
INT_PTR hex2decW(const wchar_t *wpStrHex)
{
  INT_PTR a;
  INT_PTR b=0;

  for (;;)
  {
    a=*wpStrHex++;
    if (a >= '0' && a <= '9') a-='0';
    else if (a >= 'a' && a <= 'f') a-='a' - 10;
    else if (a >= 'A' && a <= 'F') a-='A' - 10;
    else return -1;

    if (*wpStrHex) b=(b + a) * 16;
    else return (b + a);
  }
}
#endif

/********************************************************************
 *
 *  dec2hexA
 *
 *Converts decimal to hex value.
 *
 *[in]   UINT_PTR nDec        Positive integer.
 *[out]  char *szStrHex       Hex value (output), if NULL required buffer size returned in TCHARs.
 *[in]   unsigned int nWidth  Minimum number of characters to the output.
 *[in]   BOOL bLowerCase       TRUE hexadecimal value in lowercase.
 *                             FALSE in uppercase.
 *
 *Returns: copied chars.
 *
 *Examples:
 *  dec2hexA(2591, szResult, 2, FALSE);   //szResult == "A1F"
 *  dec2hexA(10, szResult, 2, TRUE);      //szResult == "0a"
 ********************************************************************/
#if defined dec2hexA || defined ALLSTRFUNC
#define dec2hexA_INCLUDED
#undef dec2hexA
int dec2hexA(UINT_PTR nDec, char *szStrHex, unsigned int nWidth, BOOL bLowerCase)
{
  UINT_PTR a=nDec;
  DWORD b=0;
  DWORD c=0;
  char d;
  int nResult;

  do
  {
    b=(DWORD)(a % 16);
    a=a / 16;
    if (b < 10)
    {
      if (szStrHex) szStrHex[c]=(char)(b + '0');
      ++c;
    }
    else
    {
      if (szStrHex) szStrHex[c]=(char)(b + (bLowerCase?'a':'A') - 10);
      ++c;
    }
  }
  while (a);

  if (!szStrHex)
    return max(nWidth, c) + 1;

  //Special form of memset implementation
  if (c < nWidth)
  {
    for (;;)
    {
      szStrHex[c]='0';
      if (++c >= nWidth) break;
    }
  }
  szStrHex[c]='\0';
  nResult=c;

  if (c)
  {
    for (b=0, --c; b < c; d=szStrHex[b], szStrHex[b++]=szStrHex[c], szStrHex[c--]=d);
  }
  return nResult;
}
#endif

/********************************************************************
 *
 *  dec2hexW
 *
 *Converts decimal to unicode hex value.
 *
 *[in]   UINT_PTR nDec        Positive integer.
 *[out]  wchar_t *wszStrHex   Unicode hex value (output), if NULL required buffer size returned in TCHARs.
 *[in]   unsigned int nWidth  Minimum number of characters to the output.
 *[in]   BOOL bLowerCase       TRUE hexadecimal value in lowercase.
 *                             FALSE in uppercase.
 *
 *Returns: copied chars.
 *
 *Examples:
 *  dec2hexW(2591, wszResult, 2, FALSE);   //wszResult == L"A1F"
 *  dec2hexW(10, wszResult, 2, TRUE);      //wszResult == L"0a"
 ********************************************************************/
#if defined dec2hexW || defined ALLSTRFUNC
#define dec2hexW_INCLUDED
#undef dec2hexW
int dec2hexW(UINT_PTR nDec, wchar_t *wszStrHex, unsigned int nWidth, BOOL bLowerCase)
{
  UINT_PTR a=nDec;
  DWORD b=0;
  DWORD c=0;
  wchar_t d;
  int nResult;

  do
  {
    b=(DWORD)(a % 16);
    a=a / 16;
    if (b < 10)
    {
      if (wszStrHex) wszStrHex[c]=(wchar_t)(b + '0');
      ++c;
    }
    else
    {
      if (wszStrHex) wszStrHex[c]=(wchar_t)(b + (bLowerCase?L'a':L'A') - 10);
      ++c;
    }
  }
  while (a);

  if (!wszStrHex)
    return max(nWidth, c) + 1;

  //Special form of memset implementation
  if (c < nWidth)
  {
    for (;;)
    {
      wszStrHex[c]=L'0';
      if (++c >= nWidth) break;
    }
  }
  wszStrHex[c]='\0';
  nResult=c;

  if (c)
  {
    for (b=0, --c; b < c; d=wszStrHex[b], wszStrHex[b++]=wszStrHex[c], wszStrHex[c--]=d);
  }
  return nResult;
}
#endif

/********************************************************************
 *
 *  bin2hexA
 *
 *Converts binary data to hex string.
 *
 *[in]   const unsigned char *pData  Binary data.
 *[in]   INT_PTR nBytes              Number of bytes in pData.
 *[out]  char *szStrHex              Output hex string buffer.
 *[in]   INT_PTR nStrHexMax          Size of the hex string buffer in TCHARs.
 *[in]   BOOL bLowerCase              TRUE hexadecimal value in lowercase.
 *                                    FALSE in uppercase.
 *
 *Returns: copied chars.
 *
 *Examples:
 *  bin2hexA((unsigned char *)"Some Text", lstrlenA("Some Text"), szResult, MAX_PATH, TRUE);   //szResult == "536f6d652054657874"
 *
 *Note:
 *  bin2hexA uses dec2hexA.
 ********************************************************************/
#if defined bin2hexA || defined ALLSTRFUNC
#define bin2hexA_INCLUDED
#undef bin2hexA
INT_PTR bin2hexA(const unsigned char *pData, INT_PTR nBytes, char *szStrHex, INT_PTR nStrHexMax, BOOL bLowerCase)
{
  INT_PTR a;
  INT_PTR b;

  nStrHexMax-=3;

  for (a=0, b=0; a < nBytes && b <= nStrHexMax; ++a, b+=2)
  {
    dec2hexA((unsigned int)pData[a], szStrHex + b, 2, bLowerCase);
  }
  szStrHex[b]='\0';
  return b;
}
#endif

/********************************************************************
 *
 *  bin2hexW
 *
 *Converts binary data to hex unicode string.
 *
 *[in]   const unsigned char *pData  Binary data.
 *[in]   INT_PTR nBytes              Number of bytes in pData.
 *[out]  wchar_t *wszStrHex          Output hex string buffer.
 *[in]   INT_PTR nStrHexMax          Size of the hex string buffer in TCHARs.
 *[in]   BOOL bLowerCase              TRUE hexadecimal value in lowercase.
 *                                    FALSE in uppercase.
 *
 *Returns: copied chars.
 *
 *Note:
 *  bin2hexW uses dec2hexW.
 *
 *Examples:
 *  bin2hexW((unsigned char *)"Some Text", lstrlenA("Some Text"), wszResult, MAX_PATH, TRUE);   //wszResult == L"536f6d652054657874"
 ********************************************************************/
#if defined bin2hexW || defined ALLSTRFUNC
#define bin2hexW_INCLUDED
#undef bin2hexW
INT_PTR bin2hexW(const unsigned char *pData, INT_PTR nBytes, wchar_t *wszStrHex, INT_PTR nStrHexMax, BOOL bLowerCase)
{
  INT_PTR a;
  INT_PTR b;

  nStrHexMax-=3;

  for (a=0, b=0; a < nBytes && b <= nStrHexMax; ++a, b+=2)
  {
    dec2hexW((unsigned int)pData[a], wszStrHex + b, 2, bLowerCase);
  }
  wszStrHex[b]='\0';
  return b;
}
#endif

/********************************************************************
 *
 *  hex2binA
 *
 *Converts hex string to binary data.
 *
 *[in]   const char *pStrHex   Hex string.
 *[out]  unsigned char *pData  Output buffer, if NULL required buffer size returned in TCHARs.
 *[in]   INT_PTR nDataMax      Size of the output buffer in bytes.
 *
 *Returns: copied bytes.
 *
 *Note:
 *  hex2binA uses hex2decA.
 *
 *Examples:
 *  hex2binA("536f6d652054657874", (unsigned char *)szResult, MAX_PATH);   //szResult == "Some Text"
 ********************************************************************/
#if defined hex2binA || defined ALLSTRFUNC
#define hex2binA_INCLUDED
#undef hex2binA
INT_PTR hex2binA(const char *pStrHex, unsigned char *pData, INT_PTR nDataMax)
{
  char szHexChar[4];
  INT_PTR nHexChar;
  UINT_PTR a;
  UINT_PTR b;

  if (!pData) nDataMax=-1;

  for (a=0, b=0; pStrHex[a] && b < (UINT_PTR)nDataMax; ++b)
  {
    szHexChar[0]=pStrHex[a++];
    if (!pStrHex[a]) break;
    szHexChar[1]=pStrHex[a++];
    szHexChar[2]='\0';

    if ((nHexChar=hex2decA(szHexChar)) >= 0)
    {
      if (pData) pData[b]=(unsigned char)nHexChar;
    }
    else break;
  }
  return b;
}
#endif

/********************************************************************
 *
 *  hex2binW
 *
 *Converts hex unicode string to binary data.
 *
 *[in]   const wchar_t *wpStrHex  Hex unicode string.
 *[out]  unsigned char *pData     Output buffer, if NULL required buffer size returned in TCHARs.
 *[in]   INT_PTR nDataMax         Size of the output buffer in bytes.
 *
 *Returns: copied bytes.
 *
 *Note:
 *  hex2binW uses hex2decW.
 *
 *Examples:
 *  hex2binW(L"536f6d652054657874", (unsigned char *)szResult, MAX_PATH);   //szResult == "Some Text"
 ********************************************************************/
#if defined hex2binW || defined ALLSTRFUNC
#define hex2binW_INCLUDED
#undef hex2binW
INT_PTR hex2binW(const wchar_t *wpStrHex, unsigned char *pData, INT_PTR nDataMax)
{
  wchar_t wszHexChar[4];
  INT_PTR nHexChar;
  UINT_PTR a;
  UINT_PTR b;

  if (!pData) nDataMax=-1;

  for (a=0, b=0; wpStrHex[a] && b < (UINT_PTR)nDataMax; ++b)
  {
    wszHexChar[0]=wpStrHex[a++];
    if (!wpStrHex[a]) break;
    wszHexChar[1]=wpStrHex[a++];
    wszHexChar[2]='\0';

    if ((nHexChar=hex2decW(wszHexChar)) >= 0)
    {
      if (pData) pData[b]=(unsigned char)nHexChar;
    }
    else break;
  }
  return b;
}
#endif

/********************************************************************
 *
 *  xprintfA
 *
 *Function formats and stores a series of characters and values in a buffer.
 *
 *[out] char *szOutput       Pointer to a buffer to receive the formatted output. If NULL required buffer size returned in TCHARs.
 *[in]  const char *pFormat  Pointer to a null-terminated string that contains the format-control specifications.
 *                            pFormat syntax is equal to wsprintfA function.
 *[in]  ...                  Specifies one or more optional arguments. The number and type of argument parameters
 *                            depend on the corresponding format-control specifications in the pFormat parameter.
 *                           %[-][0][width][.precision]type
 *                             "-"           Pad the output with blanks or zeros to the right to fill the field width, justifying output to the left.
 *                             "0"           Pad the output value with zeros to fill the field width. If this field is omitted, the output value is padded with blank spaces.
 *                             [width]       Copy the specified minimum number of characters to the output buffer.
 *                             [.precision]  For numbers, copy the specified minimum number of digits to the output buffer. If the number of digits in the argument is less than the specified precision, the output value is padded on the left with zeros.
 *                                           For strings, copy the specified maximum number of characters to the output buffer.
 *                                           Supported special format to specify argument as precision: "%.%us" or "%.%ds"
 *                             [type]
 *                               "c"         ansi character.
 *                               "d"         signed integer (32-bit).
 *                               "Id"        signed integer (32-bit on x86, 64-bit on x64).
 *                               "u"         unsigned integer (32-bit).
 *                               "Iu"        unsigned integer (32-bit on x86, 64-bit on x64).
 *                               "x","X"     unsigned hexadecimal integer in lowercase or uppercase (32-bit).
 *                               "Ix","IX"   unsigned hexadecimal integer in lowercase or uppercase (32-bit on x86, 64-bit on x64).
 *                               "s"         ansi string.
 *                               "S"         unicode string.
 *
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xprintfA uses xatoiA, xitoaA, xuitoaA, dec2hexA, xstrcpynA.
 *
 *Examples:
 *  xprintfA(szResult, "%d | %u | %x | %X | %s", -123, 123, 123, 123, "string");   //szResult == "-123 | 123 | 7b | 7B | string"
 ********************************************************************/
#if defined xprintfA || defined ALLSTRFUNC
#define xprintfA_INCLUDED
#undef xprintfA
INT_PTR xprintfA(char *szOutput, const char *pFormat, ...)
{
  const char *pFmt=pFormat;
  char *pOut=szOutput;
  char *pStartOut;
  unsigned char *pString;
  char chFillChar;
  unsigned int dwPrecision;
  unsigned int dwLen=0;
  int nWidth;
  BOOL bInt64;
  INT_PTR nSigned;
  INT_PTR nUnsigned;

  va_list val;
  va_start(val, pFormat);

  while (*pFmt)
  {
    if (*pFmt == '%')
    {
      pStartOut=pOut;
      chFillChar=' ';
      dwPrecision=0;
      nWidth=0;
      bInt64=FALSE;
      ++pFmt;

      if (*pFmt == '-')
      {
        nWidth=-1;
        ++pFmt;
      }
      if (*pFmt == '0')
      {
        chFillChar='0';
        ++pFmt;
      }
      if (*pFmt >= '1' && *pFmt <= '9')
      {
        if (nWidth == -1)
          nWidth-=(int)xatoiA(pFmt, &pFmt) - 1;
        else
          nWidth=(int)xatoiA(pFmt, &pFmt);
      }
      if (*pFmt == '.')
      {
        //Special format to specify argument as precision: "%.%us" or "%.%ds"
        if (*++pFmt == '%' && (*(pFmt + 1) == 'u' || *(pFmt + 1) == 'd'))
        {
          dwPrecision=(unsigned int)va_arg(val, INT_PTR);
          pFmt+=2;
        }
        else dwPrecision=(unsigned int)xatoiA(pFmt, &pFmt);

        if (!dwPrecision)
        {
          //"%.0s" - ignore string
          dwPrecision=(unsigned int)-1;
        }
      }
      if (*pFmt == L'I')
      {
        bInt64=TRUE;
        ++pFmt;
      }

      if (*pFmt == '%')
      {
        if (szOutput) *pOut='%';
        ++pOut;
      }
      else if (*pFmt == 'c')
      {
        nSigned=va_arg(val, INT_PTR);
        if (nWidth > 0)
        {
          nWidth=max(nWidth - 1, 0);
          pOut+=nWidth;
        }
        if (szOutput) *pOut=(char)nSigned;
        ++pOut;
      }
      else if (*pFmt == 'd')
      {
        if (bInt64)
          nSigned=va_arg(val, INT_PTR);
        else
          nSigned=va_arg(val, int);
        if (!szOutput || nWidth > 0)
        {
          dwLen=xitoaA(nSigned, NULL) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            pOut+=nWidth;
            if (nWidth > 0 && nSigned < 0)
            {
              if (szOutput) *pStartOut='-';
              ++pStartOut;
            }
          }
        }
        if (szOutput) dwLen=xitoaA(nSigned, pOut);
        pOut+=dwLen;
      }
      else if (*pFmt == 'u')
      {
        if (bInt64)
          nUnsigned=va_arg(val, UINT_PTR);
        else
          nUnsigned=va_arg(val, UINT);
        if (!szOutput || nWidth > 0)
        {
          dwLen=xuitoaA(nUnsigned, NULL) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            pOut+=nWidth;
          }
        }
        if (szOutput) dwLen=xuitoaA(nUnsigned, pOut);
        pOut+=dwLen;
      }
      else if (*pFmt == 'x' || *pFmt == 'X')
      {
        if (bInt64)
          nUnsigned=va_arg(val, UINT_PTR);
        else
          nUnsigned=va_arg(val, UINT);
        if (!szOutput || nWidth > 0)
        {
          dwLen=dec2hexA(nUnsigned, NULL, 0, (*pFmt == 'x')?TRUE:FALSE) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            pOut+=nWidth;
          }
        }
        if (szOutput) dwLen=dec2hexA(nUnsigned, pOut, 0, (*pFmt == 'x')?TRUE:FALSE);
        pOut+=dwLen;
      }
      else if (*pFmt == 's' || *pFmt == 'S')
      {
        if (pString=va_arg(val, unsigned char *))
        {
          if (dwPrecision != (DWORD)-1)
          {
            if (*pFmt == 'S')
              dwLen=lstrlenW((wchar_t *)pString);
            if (!szOutput || nWidth > 0)
            {
              if (*pFmt == 's')
                dwLen=lstrlenA((char *)pString);
              if (dwPrecision)
                dwLen=min(dwLen, dwPrecision);
              if (nWidth > 0)
              {
                nWidth=max(nWidth - (int)dwLen, 0);
                pOut+=nWidth;
              }
            }
            if (szOutput)
            {
              if (*pFmt == 'S')
              {
                if (dwLen=WideCharToMultiByte(CP_ACP, 0, (wchar_t *)pString, dwLen + 1, pOut, (dwLen + 1) * sizeof(wchar_t), NULL, NULL))
                  pOut[--dwLen]='\0';
              }
              else dwLen=(unsigned int)xstrcpynA(pOut, (char *)pString, dwPrecision?dwPrecision + 1:(unsigned int)-1);
            }
            pOut+=dwLen;
          }
        }
      }

      if (nWidth > 0)
      {
        if (szOutput)
        {
          while (--nWidth >= 0)
            pStartOut[nWidth]=chFillChar;
        }
      }
      else if (nWidth < 0)
      {
        nWidth=(int)min((pOut - pStartOut) + nWidth, 0);

        if (szOutput)
        {
          //Special form of memset implementation
          if (++nWidth <= 0)
          {
            for (;;)
            {
              *pOut=chFillChar;
              if (++nWidth > 0) break;
              ++pOut;
            }
          }
        }
        else pOut+=0 - nWidth;
      }
    }
    else
    {
      if (szOutput) *pOut=*pFmt;
      ++pOut;
    }
    ++pFmt;
  }
  if (szOutput)
    *pOut='\0';
  else
    ++pOut;

  va_end(val);
  return pOut - szOutput;
}
#endif

/********************************************************************
 *
 *  xprintfW
 *
 *Function formats and stores a series of characters and values in a buffer.
 *
 *[out] wchar_t *wszOutput       Pointer to a buffer to receive the formatted output. If NULL required buffer size returned in TCHARs.
 *[in]  const wchar_t *wpFormat  Pointer to a null-terminated string that contains the format-control specifications.
 *                                wpFormat syntax is equal to wsprintfW function.
 *[in]  ...                      Specifies one or more optional arguments. The number and type of argument parameters
 *                                depend on the corresponding format-control specifications in the wpFormat parameter.
 *                               %[-][0][width][.precision]type
 *                                 "-"           Pad the output with blanks or zeros to the right to fill the field width, justifying output to the left.
 *                                 "0"           Pad the output value with zeros to fill the field width. If this field is omitted, the output value is padded with blank spaces.
 *                                 [width]       Copy the specified minimum number of characters to the output buffer.
 *                                 [.precision]  For numbers, copy the specified minimum number of digits to the output buffer. If the number of digits in the argument is less than the specified precision, the output value is padded on the left with zeros.
 *                                               For strings, copy the specified maximum number of characters to the output buffer.
 *                                               Supported special format to specify argument as precision: "%.%us" or "%.%ds"
 *                                 [type]
 *                                   "c"         unicode character.
 *                                   "d"         signed integer (32-bit).
 *                                   "Id"        signed integer (32-bit on x86, 64-bit on x64).
 *                                   "u"         unsigned integer (32-bit).
 *                                   "Iu"        unsigned integer (32-bit on x86, 64-bit on x64).
 *                                   "x","X"     unsigned hexadecimal integer in lowercase or uppercase (32-bit).
 *                                   "Ix","IX"   unsigned hexadecimal integer in lowercase or uppercase (32-bit on x86, 64-bit on x64).
 *                                   "s"         unicode string.
 *                                   "S"         ansi string.
 *
 *
 *Returns:  number of characters copied, not including the terminating null character.
 *
 *Note:
 *  xprintfW uses xatoiW, xitoaW, xuitoaW, dec2hexW, xstrcpynW, xstrlenW.
 *
 *Examples:
 *  xprintfW(szResult, L"%d | %u | %x | %X | %s", -123, 123, 123, 123, L"string");   //szResult == "-123 | 123 | 7b | 7B | string"
 ********************************************************************/
#if defined xprintfW || defined ALLSTRFUNC
#define xprintfW_INCLUDED
#undef xprintfW
INT_PTR xprintfW(wchar_t *wszOutput, const wchar_t *wpFormat, ...)
{
  const wchar_t *wpFmt=wpFormat;
  wchar_t *wpOut=wszOutput;
  wchar_t *wpStartOut;
  unsigned char *pString;
  wchar_t wchFillChar;
  unsigned int dwPrecision;
  unsigned int dwLen=0;
  int nWidth;
  BOOL bInt64;
  INT_PTR nSigned;
  INT_PTR nUnsigned;

  va_list val;
  va_start(val, wpFormat);

  while (*wpFmt)
  {
    if (*wpFmt == L'%')
    {
      wpStartOut=wpOut;
      wchFillChar=L' ';
      dwPrecision=0;
      nWidth=0;
      bInt64=FALSE;
      ++wpFmt;

      if (*wpFmt == L'-')
      {
        nWidth=-1;
        ++wpFmt;
      }
      if (*wpFmt == L'0')
      {
        wchFillChar=L'0';
        ++wpFmt;
      }
      if (*wpFmt >= L'1' && *wpFmt <= L'9')
      {
        if (nWidth == -1)
          nWidth-=(int)xatoiW(wpFmt, &wpFmt) - 1;
        else
          nWidth=(int)xatoiW(wpFmt, &wpFmt);
      }
      if (*wpFmt == L'.')
      {
        //Special format to specify argument as precision: "%.%us" or "%.%ds"
        if (*++wpFmt == '%' && (*(wpFmt + 1) == L'u' || *(wpFmt + 1) == L'd'))
        {
          dwPrecision=(unsigned int)va_arg(val, INT_PTR);
          wpFmt+=2;
        }
        else dwPrecision=(unsigned int)xatoiW(wpFmt, &wpFmt);

        if (!dwPrecision)
        {
          //"%.0s" - ignore string
          dwPrecision=(unsigned int)-1;
        }
      }
      if (*wpFmt == L'I')
      {
        bInt64=TRUE;
        ++wpFmt;
      }

      if (*wpFmt == L'%')
      {
        if (wszOutput) *wpOut=L'%';
        ++wpOut;
      }
      else if (*wpFmt == L'c')
      {
        nSigned=va_arg(val, INT_PTR);
        if (nWidth > 0)
        {
          nWidth=max(nWidth - 1, 0);
          wpOut+=nWidth;
        }
        if (wszOutput) *wpOut=(wchar_t)nSigned;
        ++wpOut;
      }
      else if (*wpFmt == L'd')
      {
        if (bInt64)
          nSigned=va_arg(val, INT_PTR);
        else
          nSigned=va_arg(val, int);
        if (!wszOutput || nWidth > 0)
        {
          dwLen=xitoaW(nSigned, NULL) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            wpOut+=nWidth;
            if (nWidth > 0 && nSigned < 0)
            {
              if (wszOutput) *wpStartOut=L'-';
              ++wpStartOut;
            }
          }
        }
        if (wszOutput) dwLen=xitoaW(nSigned, wpOut);
        wpOut+=dwLen;
      }
      else if (*wpFmt == L'u')
      {
        if (bInt64)
          nUnsigned=va_arg(val, UINT_PTR);
        else
          nUnsigned=va_arg(val, UINT);
        if (!wszOutput || nWidth > 0)
        {
          dwLen=xuitoaW(nUnsigned, NULL) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            wpOut+=nWidth;
          }
        }
        if (wszOutput) dwLen=xuitoaW(nUnsigned, wpOut);
        wpOut+=dwLen;
      }
      else if (*wpFmt == L'x' || *wpFmt == L'X')
      {
        if (bInt64)
          nUnsigned=va_arg(val, UINT_PTR);
        else
          nUnsigned=va_arg(val, UINT);
        if (!wszOutput || nWidth > 0)
        {
          dwLen=dec2hexW(nUnsigned, NULL, 0, (*wpFmt == L'x')?TRUE:FALSE) - 1;
          if (nWidth > 0)
          {
            nWidth=max(nWidth - (int)dwLen, 0);
            wpOut+=nWidth;
          }
        }
        if (wszOutput) dwLen=dec2hexW(nUnsigned, wpOut, 0, (*wpFmt == L'x')?TRUE:FALSE);
        wpOut+=dwLen;
      }
      else if (*wpFmt == L's' || *wpFmt == L'S')
      {
        if (pString=va_arg(val, unsigned char *))
        {
          if (dwPrecision != (DWORD)-1)
          {
            if (*wpFmt == L'S')
              dwLen=lstrlenA((char *)pString);
            if (!wszOutput || nWidth > 0)
            {
              if (*wpFmt == L's')
                dwLen=lstrlenW((wchar_t *)pString);
              if (dwPrecision)
                dwLen=min(dwLen, dwPrecision);
              if (nWidth > 0)
              {
                nWidth=max(nWidth - (int)dwLen, 0);
                wpOut+=nWidth;
              }
            }
            if (wszOutput)
            {
              if (*wpFmt == L'S')
              {
                if (dwLen=MultiByteToWideChar(CP_ACP, 0, (char *)pString, dwLen + 1, wpOut, dwLen + 1))
                  wpOut[--dwLen]='\0';
              }
              else dwLen=(unsigned int)xstrcpynW(wpOut, (wchar_t *)pString, dwPrecision?dwPrecision + 1:(unsigned int)-1);
            }
            wpOut+=dwLen;
          }
        }
      }

      if (nWidth > 0)
      {
        if (wszOutput)
        {
          while (--nWidth >= 0)
            wpStartOut[nWidth]=wchFillChar;
        }
      }
      else if (nWidth < 0)
      {
        nWidth=(int)min((wpOut - wpStartOut) + nWidth, 0);

        if (wszOutput)
        {
          //Special form of memset implementation
          if (++nWidth <= 0)
          {
            for (;;)
            {
              *wpOut=wchFillChar;
              if (++nWidth > 0) break;
              ++wpOut;
            }
          }
        }
        else wpOut+=0 - nWidth;
      }
    }
    else
    {
      if (wszOutput) *wpOut=*wpFmt;
      ++wpOut;
    }
    ++wpFmt;
  }
  if (wszOutput)
    *wpOut=L'\0';
  else
    ++wpOut;

  va_end(val);
  return wpOut - wszOutput;
}
#endif

/********************************************************************
 *
 *  UTF16toUTF8
 *
 *Converts UTF-16 string to UTF-8 string.
 *
 * [in] const unsigned short *pSource  UTF-16 string.
 * [in] UINT_PTR nSourceLen            UTF-16 string length in characters.
 *[out] UINT_PTR *nSourceDone          Number of processed characters in UTF-16 string. Can be NULL.
 *[out] unsigned char *szTarget        Output buffer, if NULL required buffer size returned in bytes.
 * [in] UINT_PTR nTargetMax            Size of the output buffer in bytes.
 *
 *Returns: number of bytes copied to szTarget buffer.
 ********************************************************************/
#if defined UTF16toUTF8 || defined ALLSTRFUNC
#define UTF16toUTF8_INCLUDED
#undef UTF16toUTF8
UINT_PTR UTF16toUTF8(const unsigned short *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned char *szTarget, UINT_PTR nTargetMax)
{
  static const unsigned int lpFirstByteMark[7]={0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
  const unsigned short *pSrc=pSource;
  const unsigned short *pSrcEnd=pSource + nSourceLen;
  const unsigned short *pSrcDone=pSource;
  unsigned char *pDst;
  unsigned char *pDstEnd;
  unsigned long nChar;
  unsigned int nBitesInChar;

  if (!szTarget && !nTargetMax)
  {
    pDst=NULL;
    pDstEnd=(unsigned char *)MAXUINT_PTR;
  }
  else
  {
    pDst=szTarget;
    pDstEnd=szTarget + nTargetMax;
  }

  while (pSrc < pSrcEnd && pDst < pDstEnd)
  {
    nChar=*pSrc;

    //Surrogate pair. High surrogate.
    if (nChar >= 0xD800 && nChar <= 0xDBFF)
    {
      if (++pSrc >= pSrcEnd)
      {
        --pSrc;
        break;
      }

      //Low surrogate
      if (*pSrc >= 0xDC00 && *pSrc <= 0xDFFF)
        nChar=0x10000 + ((nChar - 0xD800) << 10) + (*pSrc - 0xDC00);
      else
      {
        pSrcDone=pSource;
        continue;
      }
    }

    if (nChar < 0x110000)
    {
      if (nChar >= 0x10000)
      {
        if (pDst + 3 >= pDstEnd) break;
        nBitesInChar=4;
      }
      else if (nChar >= 0x800)
      {
        if (pDst + 2 >= pDstEnd) break;
        nBitesInChar=3;
      }
      else if (nChar >= 0x80)
      {
        if (pDst + 1 >= pDstEnd) break;
        nBitesInChar=2;
      }
      else //if (nChar >= 0)
      {
        nBitesInChar=1;
      }

      switch (nBitesInChar)
      {
        case 4:
        {
          if (szTarget)
            *(pDst + 3)=(unsigned char)(nChar | 0x80) & 0xBF;
          nChar=nChar >> 6;
        }
        case 3:
        {
          if (szTarget)
            *(pDst + 2)=(unsigned char)(nChar | 0x80) & 0xBF;
          nChar=nChar >> 6;
        }
        case 2:
        {
          if (szTarget)
            *(pDst + 1)=(unsigned char)(nChar | 0x80) & 0xBF;
          nChar=nChar >> 6;
        }
        case 1:
        {
          if (szTarget)
            *pDst=(unsigned char)(nChar | lpFirstByteMark[nBitesInChar]);
        }
      }
      pDst+=nBitesInChar;
    }
    pSrcDone=++pSrc;
  }
  if (nSourceDone)
    *nSourceDone=pSrcDone - pSource;
  return (pDst - szTarget);
}
#endif

/********************************************************************
 *
 *  UTF8toUTF16
 *
 *Converts UTF-8 string to UTF-16 string.
 *
 * [in] const unsigned char *pSource  UTF-8 string.
 * [in] UINT_PTR nSourceLen           UTF-8 string length in bytes.
 *[out] UINT_PTR *nSourceDone         Number of processed bytes in UTF-8 string. Can be NULL.
 *[out] unsigned short *szTarget      Output buffer, if NULL required buffer size returned in characters.
 * [in] UINT_PTR nTargetMax           Size of the output buffer in characters.
 *
 *Returns: number of characters copied to szTarget buffer.
 ********************************************************************/
#if defined UTF8toUTF16 || defined ALLSTRFUNC
#define UTF8toUTF16_INCLUDED
#undef UTF8toUTF16
UINT_PTR UTF8toUTF16(const unsigned char *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned short *szTarget, UINT_PTR nTargetMax)
{
  static const unsigned int lpOffsetsFromUTF8[6]={0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080};
  const unsigned char *pSrc=pSource;
  const unsigned char *pSrcEnd=pSource + nSourceLen;
  const unsigned char *pSrcDone=pSource;
  unsigned short *pDst;
  unsigned short *pDstEnd;
  unsigned long nChar;
  unsigned int nTrailing;

  if (!szTarget && !nTargetMax)
  {
    pDst=NULL;
    pDstEnd=(unsigned short *)MAXUINT_PTR;
  }
  else
  {
    pDst=szTarget;
    pDstEnd=szTarget + nTargetMax;
  }

  while (pSrc < pSrcEnd && pDst < pDstEnd)
  {
    if (*pSrc < 0x80)
    {
      nTrailing=0;
    }
    else if (*pSrc < 0xC0)
    {
      //Trailing byte in leading position
      pSrcDone=++pSrc;
      continue;
    }
    else if (*pSrc < 0xE0)
    {
      if (pSrc + 1 >= pSrcEnd) break;
      nTrailing=1;
    }
    else if (*pSrc < 0xF0)
    {
      if (pSrc + 2 >= pSrcEnd) break;
      nTrailing=2;
    }
    else if (*pSrc < 0xF8)
    {
      if (pSrc + 3 >= pSrcEnd) break;
      nTrailing=3;
    }
    else
    {
      //No chance for this in UTF-16
      pSrcDone=++pSrc;
      continue;
    }

    //Get unicode char
    nChar=0;

    switch (nTrailing)
    {
      case 3:
      {
        nChar+=*pSrc++;
        nChar=nChar << 6;
      }
      case 2:
      {
        nChar+=*pSrc++;
        nChar=nChar << 6;
      }
      case 1:
      {
        nChar+=*pSrc++;
        nChar=nChar << 6;
      }
      case 0:
      {
        nChar+=*pSrc++;
      }
    }
    nChar-=lpOffsetsFromUTF8[nTrailing];

    //Write unicode char
    if (nChar <= 0xFFFF)
    {
      if (szTarget)
        *pDst++=(unsigned short)nChar;
      else
        pDst+=1;
    }
    else
    {
      //Surrogate pair
      if (pDst + 1 >= pDstEnd) break;
      nChar-=0x10000;

      if (szTarget)
      {
        *pDst++=(unsigned short)((nChar >> 10) + 0xD800);
        *pDst++=(unsigned short)((nChar & 0x3ff) + 0xDC00);
      }
      else pDst+=2;
    }
    pSrcDone=pSrc;
  }
  if (nSourceDone)
    *nSourceDone=pSrcDone - pSource;
  return (pDst - szTarget);
}
#endif

/********************************************************************
 *
 *  UTF32toUTF16
 *
 *Converts UTF-32 string to UTF-16 string.
 *
 * [in] const unsigned long *pSource  UTF-32 string.
 * [in] UINT_PTR nSourceLen           UTF-32 string length in characters.
 *[out] UINT_PTR *nSourceDone         Number of processed characters in UTF-32 string. Can be NULL.
 *[out] unsigned short *szTarget      Output buffer, if NULL required buffer size returned in characters.
 * [in] UINT_PTR nTargetMax           Size of the output buffer in characters.
 *
 *Returns: number of characters copied to szTarget buffer.
 ********************************************************************/
#if defined UTF32toUTF16 || defined ALLSTRFUNC
#define UTF32toUTF16_INCLUDED
#undef UTF32toUTF16
UINT_PTR UTF32toUTF16(const unsigned long *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned short *szTarget, UINT_PTR nTargetMax)
{
  const unsigned long *pSrc=pSource;
  const unsigned long *pSrcEnd=pSource + nSourceLen;
  unsigned short *pDst;
  unsigned short *pDstEnd;
  unsigned long nChar;

  if (!szTarget && !nTargetMax)
  {
    pDst=NULL;
    pDstEnd=(unsigned short *)MAXUINT_PTR;
  }
  else
  {
    pDst=szTarget;
    pDstEnd=szTarget + nTargetMax;
  }

  while (pSrc < pSrcEnd && pDst < pDstEnd)
  {
    nChar=*pSrc;

    if (nChar <= 0xFFFF)
    {
      // UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values
      if (nChar >= 0xD800 && nChar <= 0xDFFF)
      {
        if (szTarget)
          *pDst++=0xFFFD;
        else
          pDst+=1;
      }
      else
      {
        if (szTarget)
          *pDst++=(unsigned short)nChar;
        else
          pDst+=1;
      }
    }
    else if (nChar <= 0x0010FFFF)
    {
      if (pDst + 1 >= pDstEnd) break;
      nChar-=0x0010000UL;

      if (szTarget)
      {
        *pDst++=(unsigned short)((nChar >> 10) + 0xD800);
        *pDst++=(unsigned short)((nChar & 0x3FFUL) + 0xDC00);
      }
      else pDst+=2;
    }
    else
    {
      if (szTarget)
        *pDst++=0xFFFD;
      else
        pDst+=1;
    }
    ++pSrc;
  }
  if (nSourceDone)
    *nSourceDone=pSrc - pSource;
  return (pDst - szTarget);
}
#endif

/********************************************************************
 *
 *  UTF16toUTF32
 *
 *Converts UTF-16 string to UTF-32 string.
 *
 * [in] const unsigned short *pSource  UTF-16 string.
 * [in] UINT_PTR nSourceLen            UTF-16 string length in characters.
 *[out] UINT_PTR *nSourceDone          Number of processed characters in UTF-16 string. Can be NULL.
 *[out] unsigned long *szTarget        Output buffer, if NULL required buffer size returned in characters.
 * [in] UINT_PTR nTargetMax            Size of the output buffer in characters.
 *
 *Returns: number of characters copied to szTarget buffer.
 ********************************************************************/
#if defined UTF16toUTF32 || defined ALLSTRFUNC
#define UTF16toUTF32_INCLUDED
#undef UTF16toUTF32
UINT_PTR UTF16toUTF32(const unsigned short *pSource, UINT_PTR nSourceLen, UINT_PTR *nSourceDone, unsigned long *szTarget, UINT_PTR nTargetMax)
{
  const unsigned short *pSrc=pSource;
  const unsigned short *pSrcEnd=pSource + nSourceLen;
  unsigned long *pDst;
  unsigned long *pDstEnd;
  unsigned long nChar;

  if (!szTarget && !nTargetMax)
  {
    pDst=NULL;
    pDstEnd=(unsigned long *)MAXUINT_PTR;
  }
  else
  {
    pDst=szTarget;
    pDstEnd=szTarget + nTargetMax;
  }

  while (pSrc < pSrcEnd && pDst < pDstEnd)
  {
    nChar=*pSrc;

    //Surrogate pair. High surrogate.
    if (nChar >= 0xD800 && nChar <= 0xDBFF)
    {
      if (++pSrc >= pSrcEnd)
      {
        --pSrc;
        break;
      }

      //Low surrogate
      if (*pSrc >= 0xDC00 && *pSrc <= 0xDFFF)
        nChar=((nChar - 0xD800) << 10) + (*pSrc - 0xDC00) + 0x0010000UL;
      else
        continue;
    }
    if (szTarget)
      *pDst++=nChar;
    else
      pDst+=1;
    ++pSrc;
  }
  if (nSourceDone)
    *nSourceDone=pSrc - pSource;
  return (pDst - szTarget);
}
#endif


/********************************************************************
 *                                                                  *
 *                           Example                                *
 *                                                                  *
 ********************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "StrFunc.h"

//Include string functions
#define xstrcmpiA
#define xstrcmpinA
#define xstrrepA
#define xstrstrA
#define xatoiA
#define xitoaA
#include "StrFunc.h"

void main()
{
  char szResult[MAX_PATH];
  char *pStringBegin=NULL;
  char *pStringEnd=NULL;
  INT_PTR nStringLen;
  int nError;

  nError=xstrcmpiA("ABC", "abc");
  printf("nError={%d}\n", nError);

  nError=xstrcmpinA("ABCdfg", "abcxyz", 3);
  printf("nError={%d}\n", nError);

  nError=xstrrepA("ABC||dfg||HJK", -1, "||", -1, "##", -1, TRUE, szResult, &nStringLen);
  printf("szResult={%s}, nStringLen={%d}, nError={%d}\n", szResult, nStringLen, nError);

  nError=xstrstrA("ABC||dfg||HJK", -1, "Dfg", -1, FALSE, &pStringBegin, &pStringEnd);
  printf("pStringBegin={%s}, pStringEnd={%s}, nError={%d}\n", pStringBegin, pStringEnd, nError);

  nError=xatoiA("123", NULL);
  printf("nError={%d}\n", nError);

  nError=xitoaA(45, szResult);
  printf("szResult={%s}, nError={%d}\n", szResult, nError);
}

*/
