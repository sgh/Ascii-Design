/*
* This file is part of Ascii Design, an open-source cross-platform Ascii Art editor
* (C) Faster 2009 - 2013
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
*
* Contact e-mail: Faster <faster3ck@gmail.com>
*
*/

#include "mainwindowimpl.h"

MainWindowImpl::MainWindowImpl(QWidget * parent)
	: QMainWindow(parent)
{
	setupUi(this);
	
	currentDocument = "";
    m_alignment = "-x";
	
	comboFonts = new QComboBox;
	toolBar->addWidget(comboFonts);
	opt = new Options;
	
    if (!opt->optionsTest()) {
		/*On windows set auto path */
		//
		#ifdef Q_OS_WIN32
		opt->windowsAutoOptions();
		#endif
		//
		#ifdef Q_OS_LINUX
		if (!showOptionsDialog()) {
			QMessageBox::information(0, tr("Warning"),
                                 tr("Please, set correctly \"figlet path\" and \"figlet fonts path\" in order to use \"Ascii Design\"!"));
            close();
		}
		#endif
    }
	loadOptions();	// Loads inifile ".ascii-design_options.conf";
	
	fMan = new FigletManager(figletPath);
	
	connect(textEditNormal, SIGNAL(textChanged()), this, SLOT(writeText()));
	connect(comboFonts, SIGNAL(currentIndexChanged(int)), this, SLOT(writeText()));
	
	setActions();

    QActionGroup *anActionGroup = new QActionGroup(this);
    anActionGroup->addAction(actionAlign_left);
    anActionGroup->addAction(actionAlign_center);
    anActionGroup->addAction(actionAlign_right);
}

void MainWindowImpl::writeText()
{
	QString myText = textEditNormal->toPlainText();
	QString myFont = comboFonts->currentText();
    QByteArray text = fMan->makeText(myText, m_alignment, QString("%1/%2.flf").arg(fontsPath).arg(myFont));
	
    // On Haiku
    textEditFiglet->setFont(QFont("DeJaVu Sans Mono"));

    #ifdef Q_OS_LINUX
	textEditFiglet->setFont(QFont("Monospace"));
	#endif
	
	#ifdef Q_OS_WIN32
	textEditFiglet->setFont(QFont("Courier"));
    #endif
	
	textEditFiglet->setText(text);
	opt->setLastFont(myFont);
}

void MainWindowImpl::loadFonts()
{
	comboFonts->clear();
	FigletFonts *fonts = new FigletFonts;
	QStringList fontsL = fonts->getFonts(fontsPath);
	
	for (int i = 0; i < fontsL.count(); i++)
		comboFonts->addItem(fontsL.at(i).left(fontsL.at(i).size()-4));
}

void MainWindowImpl::loadOptions()
{
	/* Load figlet path*/
	figletPath = opt->figletPath();
	
	/* Load last used font */
	fontsPath = opt->fontsPath();
	loadFonts();	// Search fonts

	QString lastFont = opt->lastFont();
	int idx = 0;
	
	if (lastFont != "")
		idx = comboFonts->findText(lastFont, Qt::MatchExactly);
	comboFonts->setCurrentIndex(idx);
}

bool MainWindowImpl::showOptionsDialog()
{
	dialogOptionsImpl *dialogOpt = new dialogOptionsImpl;
	bool dlgState = dialogOpt->exec();

	loadOptions();

	return dlgState;
}

void MainWindowImpl::setActions()
{
	connect(actionOpenFile, SIGNAL(triggered()), this, SLOT(openText()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

    connect(actionAlign_left, SIGNAL(triggered()), this, SLOT(changeAlignment()));
    connect(actionAlign_center, SIGNAL(triggered()), this, SLOT(changeAlignment()));
    connect(actionAlign_right, SIGNAL(triggered()), this, SLOT(changeAlignment()));

	connect(actionConfigure, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

	connect(actionInfo, SIGNAL(triggered()), this, SLOT(showInfo()));
}

void MainWindowImpl::openText()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Text File"), QDir::homePath(), tr("Text files (*.txt *)"));
	if (!fileName.isEmpty()) {
		
			QFile file(fileName);
			if (!file.open(QFile::ReadOnly | QFile::Text)) {
			QMessageBox::warning(this, tr("Application"),
		                             tr("Cannot read file %1:\n%2.")
		                             .arg(fileName)
		                             .arg(file.errorString()));
				return;
			}
		
			QTextStream in(&file);
			QApplication::setOverrideCursor(Qt::WaitCursor);
			textEditNormal->setPlainText(in.readAll());
			QApplication::restoreOverrideCursor();

		statusBar()->showMessage(tr("File loaded"), 2000);
	}
}

bool MainWindowImpl::save()
{
    if (currentDocument.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentDocument);
    }
}

bool MainWindowImpl::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Text File"), QDir::homePath(), tr("Text files (*.txt *)"));
    if (fileName.isEmpty())
        return false;
    currentDocument = fileName;
    return saveFile(fileName);
}

bool MainWindowImpl::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEditFiglet->toPlainText();
    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindowImpl::changeAlignment()
{
    if (!this->textEditNormal->toPlainText().isEmpty()) {
        if (actionAlign_right->isChecked())
            m_alignment = "-r";
        if (actionAlign_center->isChecked())
            m_alignment = "-c";
        if (actionAlign_left->isChecked())
            m_alignment = "-x";

        writeText();
    }
}

void MainWindowImpl::showInfo()
{
	DialogInfoImpl dlg;
	dlg.exec();
}

void MainWindowImpl::openPaypalLink()
{
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=HFD8FL89SU5LU", QUrl::TolerantMode));
}
