/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kreplacedialog.h"
#include "kfinddialog_p.h"

#include <QCheckBox>
#include <QRegExp>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>

#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KMessageBox>

/**
 * we need to insert the strings after the dialog is set
 * up, otherwise QComboBox will deliver an aweful big sizeHint
 * for long replacement texts.
 */
class KReplaceDialogPrivate
{
public:
    explicit KReplaceDialogPrivate(KReplaceDialog *q)
        : q(q)
    {
    }

    void _k_slotOk();

    QStringList replaceStrings;
    KReplaceDialog *const q;
    QWidget *replaceExtension = nullptr;
    bool initialShowDone = false;
};

KReplaceDialog::KReplaceDialog(QWidget *parent, long options, const QStringList &findStrings,
                               const QStringList &replaceStrings, bool hasSelection)
    : KFindDialog(parent, options, findStrings, hasSelection, true /*create replace dialog*/),
      d(new KReplaceDialogPrivate(this))
{
    d->replaceStrings = replaceStrings;
}

KReplaceDialog::~KReplaceDialog()
{
    delete d;
}

void KReplaceDialog::showEvent(QShowEvent *e)
{
    if (!d->initialShowDone) {
        d->initialShowDone = true; // only once

        if (!d->replaceStrings.isEmpty()) {
            setReplacementHistory(d->replaceStrings);
            KFindDialog::d->replace->lineEdit()->setText(d->replaceStrings[0]);
        }
    }

    KFindDialog::showEvent(e);
}

long KReplaceDialog::options() const
{
    long options = 0;

    options = KFindDialog::options();
    if (KFindDialog::d->promptOnReplace->isChecked()) {
        options |= PromptOnReplace;
    }
    if (KFindDialog::d->backRef->isChecked()) {
        options |= BackReference;
    }
    return options;
}

QWidget *KReplaceDialog::replaceExtension() const
{
    if (!d->replaceExtension) {
        d->replaceExtension = new QWidget(KFindDialog::d->replaceGrp);
        KFindDialog::d->replaceLayout->addWidget(d->replaceExtension, 3, 0, 1, 2);
    }

    return d->replaceExtension;
}

QString KReplaceDialog::replacement() const
{
    return KFindDialog::d->replace->currentText();
}

QStringList KReplaceDialog::replacementHistory() const
{
    QStringList lst = KFindDialog::d->replace->historyItems();
    // historyItems() doesn't tell us about the case of replacing with an empty string
    if (KFindDialog::d->replace->lineEdit()->text().isEmpty()) {
        lst.prepend(QString());
    }
    return lst;
}

void KReplaceDialog::setOptions(long options)
{
    KFindDialog::setOptions(options);
    KFindDialog::d->promptOnReplace->setChecked(options & PromptOnReplace);
    KFindDialog::d->backRef->setChecked(options & BackReference);
}

void KReplaceDialog::setReplacementHistory(const QStringList &strings)
{
    if (!strings.isEmpty()) {
        KFindDialog::d->replace->setHistoryItems(strings, true);
    } else {
        KFindDialog::d->replace->clearHistory();
    }
}

void KReplaceDialogPrivate::_k_slotOk()
{
    // If regex and backrefs are enabled, do a sanity check.
    if (q->KFindDialog::d->regExp->isChecked() && q->KFindDialog::d->backRef->isChecked()) {
        QRegExp r(q->pattern());
        int caps = r.captureCount();
        QRegExp check(QStringLiteral("((?:\\\\)+)(\\d+)"));
        int p = 0;
        QString rep = q->replacement();
        while ((p = check.indexIn(rep, p)) > -1) {
            if (check.cap(1).length() % 2 && check.cap(2).toInt() > caps) {
                KMessageBox::information(q, i18n(
                                             "Your replacement string is referencing a capture greater than '\\%1', ",  caps) +
                                         (caps ?
                                          i18np("but your pattern only defines 1 capture.",
                                                "but your pattern only defines %1 captures.", caps) :
                                          i18n("but your pattern defines no captures.")) +
                                         i18n("\nPlease correct."));
                return; // abort OKing
            }
            p += check.matchedLength();
        }

    }

    q->KFindDialog::d->_k_slotOk();
    q->KFindDialog::d->replace->addToHistory(q->replacement());
}

#include "moc_kreplacedialog.cpp"
