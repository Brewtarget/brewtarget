/*======================================================================================================================
 * utils/TranslatableText.h is part of Brewtarget, and is copyright the following authors 2026:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef UTILS_TRANSLATABLETEXT_H
#define UTILS_TRANSLATABLETEXT_H
#pragma once

#include <QCoreApplication>
#include <QString>

/**
 * \brief Holds a not-yet-translated piece of text, along with its translation context, so that we can translate it at
 *        the point of use rather than at the point of construction.
 *
 *        This matters in two cases:
 *           - Objects constructed during static initialisation (eg the \c TypeLookup and \c EnumDisplayNameMapping
 *             objects in model classes).  These are created before \c QApplication exists and before any
 *             \c QTranslator has been installed, so calling \c tr() at that point always just gives back the
 *             untranslated (English) text.
 *           - Widgets that need to update their text when the user changes language (on receipt of
 *             \c QEvent::LanguageChange), and therefore need to remember what to translate.
 *
 *        For \c lupdate to find the text, it needs to be marked with \c QT_TRANSLATE_NOOP, using the same context that
 *        we store here.  Eg:
 *
 *           TranslatableText const offText{"Water", QT_TRANSLATE_NOOP("Water", "No")};
 *
 *        (\c QT_TRANSLATE_NOOP(context, text) just expands to \c text, but \c lupdate does not expand macros, so we
 *        can't hide the duplication of the context string inside a macro of our own.)
 *
 *        Both pointers must point to string literals (or other strings with static storage duration), as we do not
 *        take a copy.  This keeps the struct trivially copyable and safe to use in static initialisation.
 */
struct TranslatableText {
   char const * context;
   char const * sourceText;

   /**
    * \brief Returns the text translated into the current language
    */
   [[nodiscard]] QString translated() const {
      return QCoreApplication::translate(this->context, this->sourceText);
   }
};

#endif
