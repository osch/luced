/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#include "LanguageModeSelectors.hpp"

using namespace LucED;

void LanguageModeSelectors::append(LanguageModeSelector::Ptr selector)
{
    selectors.append(selector);
    if (selector->getFileNameRegex() != Null) {
        BasicRegex regex = selector->getFileNameRegex();
        if (regex.isValid()) {
            ovector.increaseTo(regex.getOvecSize());
        }
    }
    if (selector->getFileContentRegex() != Null) {
        BasicRegex regex = selector->getFileContentRegex();
        if (regex.isValid()) {
            ovector.increaseTo(regex.getOvecSize());
        }
    }
}

String LanguageModeSelectors::getLanguageModeNameForFileName(const String& fileName)
{
    for (int i = 0; i < selectors.getLength(); ++i)
    {
        Nullable<BasicRegex> re = selectors[i]->getFileNameRegex();
        if (re.isValid()) {
            bool matched = re.get().findMatch(fileName.toCString(), fileName.getLength(), 0, BasicRegex::MatchOptions(), ovector);
            if (matched && ovector[0] == 0 && ovector[1] == fileName.getLength()) {
                return selectors[i]->getLanguageMode();
            }
        }
    }
    return "default";
}

String LanguageModeSelectors::getLanguageModeNameForFileNameAndContent(const String& fileName, RawPtr<const ByteBuffer> fileContent)
{
    for (int i = 0; i < selectors.getLength(); ++i)
    {
        LanguageModeSelector::Ptr selector = selectors[i];
        
        bool fileNameMatched = false;
        bool contentMatched  = false;
                
        Nullable<BasicRegex> fileNameRegex = selector->getFileNameRegex();
        Nullable<BasicRegex> contentRegex  = selector->getFileContentRegex();
        
        if (fileNameRegex.isValid()) {
            bool matched = fileNameRegex.get().findMatch(fileName.toCString(), fileName.getLength(), 0, BasicRegex::MatchOptions(), ovector);
            if (matched && ovector[0] == 0 && ovector[1] == fileName.getLength()) {
                fileNameMatched = true;
            }
        }
        if (contentRegex.isValid()) {
            bool matched = contentRegex.get().findMatch((const char*)fileContent->getTotalAmount(), fileContent->getLength(), 0, BasicRegex::MatchOptions(), ovector);
            if (matched) {
                contentMatched = true;
            }
        }
        if (   (!fileNameRegex.isValid() || fileNameRegex.isValid() && fileNameMatched)
            && ( !contentRegex.isValid() ||  contentRegex.isValid() && contentMatched))
        {
            return selector->getLanguageMode();
        }
    }
    return "default";
}
