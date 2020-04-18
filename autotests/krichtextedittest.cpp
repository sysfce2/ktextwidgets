/* This file is part of the KDE libraries
    Copyright (c) 2009 Thomas McGuire <mcguire@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "krichtextedittest.h"

#include <krichtextedit.h>
#include <kcolorscheme.h>

#include <QRegularExpression>
#include <QTest>
#include <QTextCursor>
#include <QTextList>
#include <QFont>
#include <QScrollBar>

QTEST_MAIN(KRichTextEditTest)

void KRichTextEditTest::testLinebreaks()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Enter the text with keypresses, for some strange reason a normal setText() or
    // setPlainText() call doesn't do the trick
    QTest::keyClicks(&edit, QStringLiteral("a\r\r"));
    edit.setTextUnderline(true);
    QTest::keyClicks(&edit, QStringLiteral("b\r\r\rc"));
    QCOMPARE(edit.toPlainText(), QStringLiteral("a\n\nb\n\n\nc"));

    QString html = edit.toCleanHtml();
    edit.clear();
    edit.setHtml(html);
    QCOMPARE(edit.toPlainText(), QStringLiteral("a\n\nb\n\n\nc"));
}

void KRichTextEditTest::testUpdateLinkAdd()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QStringLiteral("Test"));
    QTextCharFormat charFormat = cursor.charFormat();
    // Note that QTextEdit doesn't use the palette. Black is black.
    QCOMPARE(charFormat.foreground().color().name(), QColor(Qt::black).name());

    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QStringLiteral("http://www.kde.org"), QStringLiteral("KDE"));

    // Validate text and formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QStringLiteral("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), true);
    charFormat = cursor.charFormat();
    QCOMPARE(charFormat.foreground(), QBrush(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color()));
    QCOMPARE(charFormat.underlineColor(), KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::SingleUnderline);
}

void KRichTextEditTest::testUpdateLinkRemove()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QStringLiteral("Test"));
    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QStringLiteral("http://www.kde.org"), QStringLiteral("KDE"));

    // Remove link and validate formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    edit.updateLink(QString(), QStringLiteral("KDE"));
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QStringLiteral("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), false);
    QTextCharFormat charFormat = cursor.charFormat();
    QCOMPARE(charFormat.foreground().color().name(), QColor(Qt::black).name());
    QCOMPARE(charFormat.underlineColor().name(), QColor(Qt::black).name());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::NoUnderline);
}

void KRichTextEditTest::testHTMLLineBreaks()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Create the following text:
    //A
    //
    //B
    QTest::keyClicks(&edit, QStringLiteral("a\r"));

    edit.setTextUnderline(true);

    QTest::keyClicks(&edit, QStringLiteral("\rb"));

    QString html = edit.toCleanHtml();

    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html line breaks.
    // For now, we'll parse the 6th line (the empty one) and make sure it has the proper format
    // The first four (4) HTML code lines are DOCTYPE through <body> declaration

    const QStringList lines = html.split(QLatin1Char('\n'));

//  for (int idx=0; idx<lines.size(); idx++) {
//    qDebug() << ( idx + 1 ) << QString( " : " ) << lines.at( idx );
//  }

    QVERIFY2(lines.size() == 7,  "we can't perform this unit test: "
             "the html rendering has changed beyond recognition");

    const QString &line6 = lines.at(5);

    // make sure that this is an empty <p> line
    QVERIFY(line6.startsWith(QStringLiteral("<p style=\"-qt-paragraph-type:empty;")));

    // make sure that empty lines have the &nbsp; inserted
    QVERIFY2(line6.endsWith(QStringLiteral(">&nbsp;</p>")), "Empty lines must have &nbsp; or otherwise 3rd party "
             "viewers render those as non-existing lines");

}

void KRichTextEditTest::testHTMLOrderedLists()
{

    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html lists.
    // For now, we'll parse the 6th line (the <ol> element) and make sure it has the proper format

    KRichTextEdit edit;
    edit.enableRichTextMode();

    edit.setTextUnderline(true);

    // create a numbered (ordered) list
    QTextCursor cursor = edit.textCursor();
    cursor.insertList(QTextListFormat::ListDecimal);

    QTest::keyClicks(&edit, QStringLiteral("a\rb\rc\r"));

    QString html = edit.toCleanHtml();

    const QStringList lines = html.split(QLatin1Char('\n'));

//  Uncomment this section in case the first test fails to see if the HTML
//  rendering has actually introduced a bug, or merely a problem with the unit test itself
//
//  for (int idx=0; idx<lines.size(); idx++) {
//    qDebug() << ( idx + 1 ) << QString( " : " ) << lines.at( idx );
//  }

    QVERIFY2(lines.size() == 9,  "we can't perform this unit test: "
             "the html rendering has changed beyond recognition");

    // this is the <ol> declaration line
    const QString &line6 = lines.at(5);

//  qDebug() << line6;

    // there should not be a margin-left: 0 defined for the <ol> element
    QRegExp regex(QStringLiteral("<ol.*margin-left: 0px.*><li"));
    regex.setMinimal(true);
    QVERIFY2(regex.indexIn(line6, 0) == -1, "margin-left: 0px specified for ordered lists "
             "removes numbers in 3rd party viewers ");

    const QRegularExpression re(QStringLiteral("<ol.*?margin-left: 0px.*?><li"));
    QVERIFY2(!re.match(line6, 0).hasMatch(), "margin-left: 0px specified for ordered lists "
                                             "removes numbers in 3rd party viewers ");

}

void KRichTextEditTest::testHTMLUnorderedLists()
{
    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html lists.
    // For now, we'll parse the 6th line (the <ul> element) and make sure it has the proper format
    // The first four (4) HTML code lines are DOCTYPE through <body> declaration

    KRichTextEdit edit;
    edit.enableRichTextMode();

    edit.setTextUnderline(true);

    // create a numbered (ordered) list
    QTextCursor cursor = edit.textCursor();
    cursor.insertList(QTextListFormat::ListDisc);

    QTest::keyClicks(&edit, QStringLiteral("a\rb\rc\r"));

    QString html = edit.toCleanHtml();

    const QStringList lines = html.split(QLatin1Char('\n'));

//  Uncomment this section in case the first test fails to see if the HTML
//  rendering has actually introduced a bug, or merely a problem with the unit test itself
//
//  for (int idx=0; idx<lines.size(); idx++) {
//    qDebug() << ( idx + 1 ) << QString( " : " ) << lines.at( idx );
//  }

    QVERIFY2(lines.size() == 9,  "we can't perform this unit test: "
             "the html rendering has changed beyond recognition");

    // this is the <ol> declaration line
    const QString &line6 = lines.at(5);

//  qDebug() << line6;

    // there should not be a margin-left: 0 defined for the <ol> element
    QRegExp regex(QStringLiteral("<ul.*margin-left: 0px.*><li"));
    regex.setMinimal(true);
    QVERIFY2(regex.indexIn(line6, 0) == -1, "margin-left: 0px specified for unordered lists "
             "removes numbers in 3rd party viewers ");

    const QRegularExpression re(QStringLiteral("<ul.*?margin-left: 0px.*?><li"));
    QVERIFY2(!re.match(line6, 0).hasMatch(), "margin-left: 0px specified for unordered lists "
                                             "removes numbers in 3rd party viewers ");
}

void KRichTextEditTest::testHeading()
{
    KRichTextEdit edit;
    // Create two lines, make second one a heading
    QTest::keyClicks(&edit, QStringLiteral("a\rb"));
    edit.setHeadingLevel(2);
    QTest::keyClicks(&edit, QStringLiteral("cd"));
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Now add a new line, make sure it's no longer a heading
    QTest::keyClicks(&edit, QStringLiteral("\ref"));
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));
    // Go to beginning of this line, press Backspace -> lines should be merged,
    // current line should become a heading
    QTest::keyClick(&edit, Qt::Key_Home);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // The line is now "bcd|ef", "|" is cursor. Press Enter, the second line should remain a heading
    QTest::keyClick(&edit, Qt::Key_Return);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Change the heading level back to normal
    edit.setHeadingLevel(0);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));
    // Go to end of previous line, press Delete -> lines should be merged again
    QTextCursor cursor = edit.textCursor();
    cursor.movePosition(QTextCursor::PreviousBlock);
    cursor.movePosition(QTextCursor::EndOfBlock);
    edit.setTextCursor(cursor);
    QTest::keyClick(&edit, Qt::Key_Delete);
    cursor.movePosition(QTextCursor::EndOfBlock);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));

    // Now playing with selection. The contents are currently:
    // ---
    // a
    // bcdef
    // ---
    // Let's add a new line 'gh', select everything between "c" and "g"
    // and change heading. It should apply to both lines
    QTest::keyClicks(&edit, QStringLiteral("\rgh"));
    cursor.setPosition(4, QTextCursor::MoveAnchor);
    cursor.setPosition(9, QTextCursor::KeepAnchor);
    edit.setTextCursor(cursor);
    edit.setHeadingLevel(5);
    // In the end, both lines should change the heading (even before the selection, i.e. 'b'!)
    cursor.setPosition(3);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 5);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // (and after the selection, i.e. 'f'!)
    cursor.setPosition(10);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 5);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
}

void KRichTextEditTest::testRulerScroll()
{
    // This is a test for bug 195828
    KRichTextEdit edit;
    // Add some lines, so that scroll definitely appears
    for (int i = 0; i < 100; i++) {
        QTest::keyClicks(&edit, QStringLiteral("New line\r"));
    }
    // Widget has to be shown for the scrollbar to be adjusted
    edit.show();
    // Ensure the scrollbar actually appears
    QVERIFY(edit.verticalScrollBar()->value() > 0);

    edit.insertHorizontalRule();
    // Make sure scrollbar didn't jump to the top
    QVERIFY(edit.verticalScrollBar()->value() > 0);
}
