#ifndef STANDARDEDITACTIONS_H
#define STANDARDEDITACTIONS_H

#include "TextEditorWidget.h"
#include "HeapObject.h"

namespace LucED {

class StandardEditActions : public HeapObject
{
public:
    typedef OwningPtr<StandardEditActions> Ptr;
    
    static Ptr createSingleLineActions(TextEditorWidget* editWidget) {
        Ptr rslt(new StandardEditActions(editWidget));
        rslt->registerSingleLineEditActionsToEditWidget();
        return rslt;
    }
    static Ptr createMultiLineActions(TextEditorWidget* editWidget) {
        Ptr rslt(new StandardEditActions(editWidget));
        rslt->registerMultiLineEditActionsToEditWidget();
        return rslt;
    }
    
    void cursorLeft();
    void cursorRight();
    void cursorDown();
    void cursorUp();
    void cursorPageDown();
    void cursorPageUp();
    void cursorBeginOfLine();
    void cursorEndOfLine();
    void scrollDown();
    void scrollUp();
    void scrollLeft();
    void scrollRight();
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();
    void cursorBeginOfText();
    void cursorEndOfText();
    void newLine();
    void backSpace();
    void deleteKey();
    void copyToClipboard();
    void pasteFromClipboard();
    void selectAll();
    void selectionCursorLeft();
    void selectionCursorRight();
    void selectionCursorDown();
    void selectionCursorUp();
    void cursorWordLeft();
    void cursorWordRight();
    void selectionCursorWordLeft();
    void selectionCursorWordRight();
    void selectionCursorBeginOfLine();
    void selectionCursorEndOfLine();
    void selectionCursorPageDown();
    void selectionCursorPageUp();
    
    
private:
    StandardEditActions(TextEditorWidget *editWidget);
    
    void registerSingleLineEditActionsToEditWidget();
    void registerMultiLineEditActionsToEditWidget();

    WeakPtr<TextEditorWidget> e;
};


} // namespace LucED


#endif // STANDARDEDITACTIONS_H
