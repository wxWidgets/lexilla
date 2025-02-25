// Scintilla source code edit control
/** @file LexSmalltalk.cxx
 ** Lexer for Smalltalk language.
 ** Written by Sergey Philippov, sphilippov-at-gmail-dot-com
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "LexCharacterSet.h"
#include "LexerModule.h"

using namespace Lexilla;

/*
| lexTable classificationBlock charClasses |
charClasses := #(#DecDigit #Letter #Special #Upper #BinSel).
lexTable := ByteArray new: 128.
classificationBlock := [ :charClass :chars |
    | flag |
    flag := 1 bitShift: (charClasses indexOf: charClass) - 1.
    chars do: [ :char | lexTable at: char codePoint + 1 put: ((lexTable at: char codePoint + 1) bitOr: flag)]].

classificationBlock
    value: #DecDigit value: '0123456789';
    value: #Letter value: '_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    value: #Special value: '()[]{};.^:';
    value: #BinSel value: '~@%&*-+=|\/,<>?!';
    value: #Upper value: 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.

((String new: 500) streamContents: [ :stream |
    stream crLf; nextPutAll: 'static int ClassificationTable[256] = {'.
    lexTable keysAndValuesDo: [ :index :value |
        ((index - 1) rem: 16) == 0 ifTrue: [
            stream crLf; tab]
        ifFalse: [
            stream space].
        stream print: value.
        index ~= 256 ifTrue: [
            stream nextPut: $,]].
    stream crLf; nextPutAll: '};'; crLf.

    charClasses keysAndValuesDo: [ :index :name |
        stream
            crLf;
            nextPutAll: (
                ('static inline bool is<1s>(int ch) {return (ch > 0) && (ch %< 0x80) && ((ClassificationTable[ch] & <2p>) != 0);}')
                    expandMacrosWith: name with: (1 bitShift: (index - 1)))
    ]]) edit
*/

// autogenerated {{{{

static int ClassificationTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 16, 0, 0, 0, 16, 16, 0, 4, 4, 16, 16, 16, 16, 4, 16,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 16, 16, 16, 16,
    16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 4, 16, 4, 4, 2,
    0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 16, 4, 16, 0,
};

static inline bool isDecDigit(int ch) {return (ch > 0) && (ch < 0x80) && ((ClassificationTable[ch] & 1) != 0);}
static inline bool isLetter(int ch) {return (ch > 0) && (ch < 0x80) && ((ClassificationTable[ch] & 2) != 0);}
static inline bool isSpecial(int ch) {return (ch > 0) && (ch < 0x80) && ((ClassificationTable[ch] & 4) != 0);}
static inline bool isUpper(int ch) {return (ch > 0) && (ch < 0x80) && ((ClassificationTable[ch] & 8) != 0);}
static inline bool isBinSel(int ch) {return (ch > 0) && (ch < 0x80) && ((ClassificationTable[ch] & 16) != 0);}
// autogenerated }}}}

static inline bool isAlphaNumeric(int ch) {
    return isDecDigit(ch) || isLetter(ch);
}

static inline bool isDigitOfRadix(int ch, int radix)
{
    if (isDecDigit(ch))
        return (ch - '0') < radix;
    else if (!isUpper(ch))
        return false;
    else
        return (ch - 'A' + 10) < radix;
}

static inline void skipComment(StyleContext& sc)
{
    while (sc.More() && sc.ch != '\"')
        sc.Forward();
}

static inline void skipString(StyleContext& sc)
{
    while (sc.More()) {
        if (sc.ch == '\'') {
            if (sc.chNext != '\'')
                return;
            sc.Forward();
        }
        sc.Forward();
    }
}

static void handleHash(StyleContext& sc)
{
    if (isSpecial(sc.chNext)) {
        sc.SetState(SCE_ST_SPECIAL);
        return;
    }

    sc.SetState(SCE_ST_SYMBOL);
    sc.Forward();
    if (sc.ch == '\'') {
        sc.Forward();
        skipString(sc);
    }
    else {
        if (isLetter(sc.ch)) {
            while (isAlphaNumeric(sc.chNext) || sc.chNext == ':')
                sc.Forward();
        }
        else if (isBinSel(sc.ch)) {
            while (isBinSel(sc.chNext))
                sc.Forward();
        }
    }
}

static inline void handleSpecial(StyleContext& sc)
{
    if (sc.ch == ':' && sc.chNext == '=') {
        sc.SetState(SCE_ST_ASSIGN);
        sc.Forward();
    }
    else {
        if (sc.ch == '^')
            sc.SetState(SCE_ST_RETURN);
        else
            sc.SetState(SCE_ST_SPECIAL);
    }
}

static inline void skipInt(StyleContext& sc, int radix)
{
    while (isDigitOfRadix(sc.chNext, radix))
        sc.Forward();
}

static void handleNumeric(StyleContext& sc)
{
    char num[256];
    int nl;
    int radix;

    sc.SetState(SCE_ST_NUMBER);
    num[0] = static_cast<char>(sc.ch);
    nl = 1;
    while (isDecDigit(sc.chNext)) {
        num[nl++] = static_cast<char>(sc.chNext);
        sc.Forward();
        if (nl+1 == sizeof(num)/sizeof(num[0])) // overrun check
            break;
    }
    if (sc.chNext == 'r') {
        num[nl] = 0;
        if (num[0] == '-')
            radix = atoi(num + 1);
        else
            radix = atoi(num);
        sc.Forward();
        if (sc.chNext == '-')
            sc.Forward();
        skipInt(sc, radix);
    }
    else
        radix = 10;
    if (sc.chNext == '.') {
        if (!isDigitOfRadix(sc.GetRelative(2), radix))
            return;
        sc.Forward();
        skipInt(sc, radix);
    }
    
    if (sc.chNext == 's') {
        // ScaledDecimal
        sc.Forward();
        while (isDecDigit(sc.chNext))
            sc.Forward();
        return;
    }
    else if (sc.chNext != 'e' && sc.chNext != 'd' && sc.chNext != 'q')
        return;
    sc.Forward();
    if (sc.chNext == '+' || sc.chNext == '-')
        sc.Forward();
    skipInt(sc, radix);
}

static inline void handleBinSel(StyleContext& sc)
{
    sc.SetState(SCE_ST_BINARY);
    while (isBinSel(sc.chNext))
        sc.Forward();
}

static void handleLetter(StyleContext& sc, WordList* specialSelectorList)
{
    char ident[256];
    int il;
    int state;
    bool doubleColonPresent;

    sc.SetState(SCE_ST_DEFAULT);

    ident[0] = static_cast<char>(sc.ch);
    il = 1;
    while (isAlphaNumeric(sc.chNext)) {
        ident[il++] = static_cast<char>(sc.chNext);
        sc.Forward();
        if (il+2 == sizeof(ident)/sizeof(ident[0])) // overrun check
            break;
    }

    if (sc.chNext == ':') {
        doubleColonPresent = true;
        ident[il++] = ':';
        sc.Forward();
    }
    else
        doubleColonPresent = false;
    ident[il] = 0;

    if (specialSelectorList->InList(ident))
            state = SCE_ST_SPEC_SEL;
    else if (doubleColonPresent)
            state = SCE_ST_KWSEND;
    else if (isUpper(ident[0]))
        state = SCE_ST_GLOBAL;
    else {
        if (!strcmp(ident, "self"))
            state = SCE_ST_SELF;
        else if (!strcmp(ident, "super"))
            state = SCE_ST_SUPER;
        else if (!strcmp(ident, "nil"))
            state = SCE_ST_NIL;
        else if (!strcmp(ident, "true") || !strcmp(ident, "false"))
            state = SCE_ST_BOOL;
        else
            state = SCE_ST_DEFAULT;
    }

    sc.ChangeState(state);
}

static void colorizeSmalltalkDoc(Sci_PositionU startPos, Sci_Position length, int initStyle, WordList *wordLists[], Accessor &styler)
{
    StyleContext sc(startPos, length, initStyle, styler);

    if (initStyle == SCE_ST_COMMENT) {
        skipComment(sc);
        if (sc.More())
            sc.Forward();
    }
    else if (initStyle == SCE_ST_STRING) {
        skipString(sc);
        if (sc.More())
            sc.Forward();
    }

    for (; sc.More(); sc.Forward()) {
        int ch;

        ch = sc.ch;
        if (ch == '\"') {
            sc.SetState(SCE_ST_COMMENT);
            sc.Forward();
            skipComment(sc);
        }
        else if (ch == '\'') {
            sc.SetState(SCE_ST_STRING);
            sc.Forward();
            skipString(sc);
        }
        else if (ch == '#')
            handleHash(sc);
        else if (ch == '$') {
            sc.SetState(SCE_ST_CHARACTER);
            sc.Forward();
        }
        else if (isSpecial(ch))
            handleSpecial(sc);
        else if (isDecDigit(ch))
            handleNumeric(sc);
        else if (isLetter(ch))
            handleLetter(sc, wordLists[0]);
        else if (isBinSel(ch)) {
            if (ch == '-' && isDecDigit(sc.chNext))
                handleNumeric(sc);
            else
                handleBinSel(sc);
        }
        else
            sc.SetState(SCE_ST_DEFAULT);
    }
    sc.Complete();
}

static const char* const smalltalkWordListDesc[] = {
    "Special selectors",
    0
};

extern const LexerModule lmSmalltalk(SCLEX_SMALLTALK, colorizeSmalltalkDoc, "smalltalk", NULL, smalltalkWordListDesc);
