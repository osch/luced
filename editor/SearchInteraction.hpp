/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
//
//   This program is free software; you can redistribute it and/or modify it
//   under the terms of the GNU General Public License Version 2 as published
//   by the Free Software Foundation in June 1991.
//
//   This program is distributed in the hope that it will be useful, but WITHOUT
//   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//   more details.
//
//   You should have received a copy of the GNU General Public License along with 
//   this program; if not, write to the Free Software Foundation, Inc., 
//   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/////////////////////////////////////////////////////////////////////////////////////

#ifndef SEARCH_INTERACTION_HPP
#define SEARCH_INTERACTION_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "String.hpp"
#include "TextData.hpp"
#include "ReplaceUtil.hpp"
#include "SearchParameter.hpp"
#include "Callback.hpp"
#include "TextEditorWidget.hpp"
#include "MessageBoxParameter.hpp"
#include "MessageBoxQueue.hpp"

namespace LucED
{

class SearchInteraction : public HeapObject
{
public:
    typedef OwningPtr<SearchInteraction> Ptr;
    typedef SearchInteraction            ThisClass;
    
    class Callbacks
    {
    public:
        Callbacks(Callback<const MessageBoxParameter&>::Ptr        messageBoxInvoker,
                  MessageBoxQueue::Ptr                             messageBoxQueue,
                  Callback<ThisClass*>::Ptr                        requestCloseOfInvokingPanel,
                  Callback<ThisClass*,Callback<String>::Ptr>::Ptr  requestCurrentSelectionCallback,
                  Callback<>::Ptr                                  exceptionHandler)
            : messageBoxInvoker(messageBoxInvoker),
              messageBoxQueue(messageBoxQueue),
              requestCloseOfInvokingPanel(requestCloseOfInvokingPanel),
              requestCurrentSelectionCallback(requestCurrentSelectionCallback),
              exceptionHandler(exceptionHandler)
        {}
    private:
        friend class SearchInteraction;
        
        Callback<const MessageBoxParameter&>::Ptr       messageBoxInvoker;
        MessageBoxQueue::Ptr                            messageBoxQueue;
        Callback<ThisClass*>::Ptr                       requestCloseOfInvokingPanel;
        Callback<ThisClass*,Callback<String>::Ptr>::Ptr requestCurrentSelectionCallback;
        Callback<>::Ptr                                 exceptionHandler;
    };
    
    static Ptr create(const SearchParameter& p, TextEditorWidget* e,
                                                const Callbacks& cb)
    {
        return Ptr(new SearchInteraction(p, e, cb));
    }
    
    void startFindSelection() {
        internalStartFind(true);
    }

    void startFind() {
        internalStartFind(false);
    }
    
    void replaceAndDontContinueWithFind() {
        internalReplaceAndFind(false);
    }
    
    void replaceAndContinueWithFind() {
        internalReplaceAndFind(false);
    }
    
    String getFindString() const {
        return replaceUtil.getFindString();
    }
    
    String getReplaceString() const {
        return replaceUtil.getReplaceString();
    }
    String getSubstitutedString() {
        return replaceUtil.getSubstitutedString();
    }

    bool replaceAllBetween(long spos, long epos) {
        return replaceUtil.replaceAllBetween(spos, epos);
    }

private:
    SearchInteraction(const SearchParameter& p, TextEditorWidget* e,
                                                const Callbacks& cb)
        : p(p),
          e(e),
          cb(cb),
          textData(e->getTextData()),
          replaceUtil(textData)
    {}
    
    void setTextPositionForSearchStart();

    void continueWithFindSelection(String currentSelection, bool autoContinue);    
    void continueWithFindSelectionWithoutAutoContinue(String currentSelection);    
    void continueWithFindSelectionAndAutoContinue(String currentSelection);
    
    void findSelectionForward();
    void findSelectionForwardAndAutoContinue();
    void findSelectionBackward();
    void findSelectionBackwardAndAutoContinue();
    
    void internalStartFind(bool findSelection);
    void internalExecute(bool isWrapping, bool autoContinue);
    
    void internalReplaceAndFind(bool continueWithFind);
    
    void handleContinueAtBeginButton();
    void handleContinueAtEndButton();
    
    void findAgainForward();
    void findAgainForwardAndAutoContinue();
    void findAgainBackward();
    void findAgainBackwardAndAutoContinue();
    
    SearchParameter p;
    RawPtr<TextEditorWidget> e;
    
    Callbacks cb;

    TextData::Ptr textData;
    ReplaceUtil replaceUtil;
};

} // namespace LucED

#endif // SEARCH_INTERACTION_HPP
