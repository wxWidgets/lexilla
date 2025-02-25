// Scintilla source code edit control
/** @file LexMake.cxx
 ** Lexer for make files.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdarg>

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

static inline bool AtEOL(Accessor &styler, Sci_PositionU i) {
	return (styler[i] == '\n') ||
	       ((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

static void ColouriseMakeLine(
	const std::string &lineBuffer,
    Sci_PositionU startLine,
    Sci_PositionU endPos,
    Accessor &styler) {

	const Sci_PositionU lengthLine = lineBuffer.length();
	Sci_PositionU i = 0;
	Sci_Position lastNonSpace = -1;
	unsigned int state = SCE_MAKE_DEFAULT;
	bool bSpecial = false;

	// check for a tab character in column 0 indicating a command
	bool bCommand = false;
	if ((lengthLine > 0) && (lineBuffer[0] == '\t'))
		bCommand = true;

	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i])) {
		i++;
	}
	if (i < lengthLine) {
		if (lineBuffer[i] == '#') {	// Comment
			styler.ColourTo(endPos, SCE_MAKE_COMMENT);
			return;
		}
		if (lineBuffer[i] == '!') {	// Special directive
			styler.ColourTo(endPos, SCE_MAKE_PREPROCESSOR);
			return;
		}
	}
	int varCount = 0;
	while (i < lengthLine) {
		if (((i + 1) < lengthLine) && (lineBuffer[i] == '$' && lineBuffer[i + 1] == '(')) {
			styler.ColourTo(startLine + i - 1, state);
			state = SCE_MAKE_IDENTIFIER;
			varCount++;
		} else if (state == SCE_MAKE_IDENTIFIER && lineBuffer[i] == ')') {
			if (--varCount == 0) {
				styler.ColourTo(startLine + i, state);
				state = SCE_MAKE_DEFAULT;
			}
		}

		// skip identifier and target styling if this is a command line
		if (!bSpecial && !bCommand) {
			if (lineBuffer[i] == ':') {
				if (((i + 1) < lengthLine) && (lineBuffer[i + 1] == '=')) {
					// it's a ':=', so style as an identifier
					if (lastNonSpace >= 0)
						styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_IDENTIFIER);
					styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
					styler.ColourTo(startLine + i + 1, SCE_MAKE_OPERATOR);
				} else {
					// We should check that no colouring was made since the beginning of the line,
					// to avoid colouring stuff like /OUT:file
					if (lastNonSpace >= 0)
						styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_TARGET);
					styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
					styler.ColourTo(startLine + i, SCE_MAKE_OPERATOR);
				}
				bSpecial = true;	// Only react to the first ':' of the line
				state = SCE_MAKE_DEFAULT;
			} else if (lineBuffer[i] == '=') {
				if (lastNonSpace >= 0)
					styler.ColourTo(startLine + lastNonSpace, SCE_MAKE_IDENTIFIER);
				styler.ColourTo(startLine + i - 1, SCE_MAKE_DEFAULT);
				styler.ColourTo(startLine + i, SCE_MAKE_OPERATOR);
				bSpecial = true;	// Only react to the first '=' of the line
				state = SCE_MAKE_DEFAULT;
			}
		}
		if (!isspacechar(lineBuffer[i])) {
			lastNonSpace = i;
		}
		i++;
	}
	if (state == SCE_MAKE_IDENTIFIER) {
		styler.ColourTo(endPos, SCE_MAKE_IDEOL);	// Error, variable reference not ended
	} else {
		styler.ColourTo(endPos, SCE_MAKE_DEFAULT);
	}
}

static void ColouriseMakeDoc(Sci_PositionU startPos, Sci_Position length, int, WordList *[], Accessor &styler) {
	std::string lineBuffer;
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	Sci_PositionU startLine = startPos;
	for (Sci_PositionU i = startPos; i < startPos + length; i++) {
		lineBuffer.push_back(styler[i]);
		if (AtEOL(styler, i)) {
			// End of line (or of line buffer) met, colourise it
			ColouriseMakeLine(lineBuffer, startLine, i, styler);
			lineBuffer.clear();
			startLine = i + 1;
		}
	}
	if (!lineBuffer.empty()) {	// Last line does not have ending characters
		ColouriseMakeLine(lineBuffer, startLine, startPos + length - 1, styler);
	}
}

static const char *const emptyWordListDesc[] = {
	nullptr
};

extern const LexerModule lmMake(SCLEX_MAKEFILE, ColouriseMakeDoc, "makefile", nullptr, emptyWordListDesc);
