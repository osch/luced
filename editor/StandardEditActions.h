#ifndef STANDARDEDITACTIONS_H
#define STANDARDEDITACTIONS_H

#include "TextEditorWidget.h"

namespace LucED {

namespace StandardEditActions
{
   typedef void (EditActionFunction)(TextEditorWidget *);

    EditActionFunction cursorLeft;
    EditActionFunction cursorRight;
    EditActionFunction cursorDown;
    EditActionFunction cursorUp;
    EditActionFunction cursorPageDown;
    EditActionFunction cursorPageUp;
    EditActionFunction cursorBeginOfLine;
    EditActionFunction cursorEndOfLine;
    EditActionFunction scrollDown;
    EditActionFunction scrollUp;
    EditActionFunction scrollLeft;
    EditActionFunction scrollRight;
    EditActionFunction scrollPageUp;
    EditActionFunction scrollPageDown;
    EditActionFunction scrollPageLeft;
    EditActionFunction scrollPageRight;
    EditActionFunction cursorBeginOfText;
    EditActionFunction cursorEndOfText;
    EditActionFunction newLine;
    EditActionFunction backSpace;
    EditActionFunction deleteKey;
    EditActionFunction copyToClipboard;
    EditActionFunction pasteFromClipboard;
    EditActionFunction selectAll;
    EditActionFunction selectionCursorLeft;
    EditActionFunction selectionCursorRight;
    EditActionFunction selectionCursorDown;
    EditActionFunction selectionCursorUp;
    EditActionFunction cursorWordLeft;
    EditActionFunction cursorWordRight;
    EditActionFunction selectionCursorWordLeft;
    EditActionFunction selectionCursorWordRight;
    EditActionFunction selectionCursorBeginOfLine;
    EditActionFunction selectionCursorEndOfLine;
    EditActionFunction selectionCursorPageDown;
    EditActionFunction selectionCursorPageUp;
    
    
    void addSingleLineEditActions(TextEditorWidget* editor);
    void addMultiLineEditActions(TextEditorWidget* editor);
};


} // namespace LucED


#endif // STANDARDEDITACTIONS_H
