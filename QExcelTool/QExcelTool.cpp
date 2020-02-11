#include "QExcelTool.h"
#include <QFile>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFileDialog>

int err = 0, ftype = -1, ntvline = 0, ntvrow = 0;
QStatusBar* xstatusBar = nullptr;
QFile* xfile = nullptr;
QTableView* xtableView = nullptr;
std::string content;


QExcelTool::QExcelTool(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.statusBar->setSizeGripEnabled(false);
	ui.tableView->setShowGrid(true);
	fxls = nullptr;
	connect(ui.pushButton_1, SIGNAL(clicked()), this, SLOT(openxlsfile()));
	xstatusBar = ui.statusBar;
	ui.statusBar->showMessage(QString::fromLocal8Bit("就绪"));

	QStandardItemModel* model = new QStandardItemModel();
	QStringList labels = QString::fromLocal8Bit("频率,功率,误差").simplified().split(",");
	model->setHorizontalHeaderLabels(labels);
	ui.tableView->setModel(model);
	ui.tableView->show();
}

void QExcelTool::openxlsfile()
{
	fpath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开表格文件"), ".", QString::fromLocal8Bit("表格文件(*.csv)")).replace("/", "\\");
	ui.textBrowser->setText(fpath);
	if (fpath.isEmpty())
	{
		ui.statusBar->showMessage(QString::fromLocal8Bit("错误：未选择文件"));
		err = QETERROR_NOFILESELECTED;
	}
	else
	{
		fxls = new QFile(fpath);
		fxls->open(QIODevice::ReadOnly | QIODevice::Text);
		if (!fxls->isOpen())
		{
			ui.statusBar->showMessage(QString::fromLocal8Bit("错误：打开文件失败（%1)").arg(fxls->errorString()));
			err = QETERROR_FILEOPENFAIL;
		}
		else
		{
			std::string spath;
			spath = fpath.toStdString();
			spath = spath.substr(spath.find_last_of('\\') + 1);
			xstatusBar->showMessage(QString::fromLocal8Bit("信息：打开文件成功\t文件名：\"%1\"").arg(QString(spath.c_str())));
			spath = fpath.toStdString();
			spath = spath.substr(spath.find_last_of('.') + 1);
			ftype = (spath == "csv" ? QETFILETYPE_CSV : (spath == "xls" ? QETFILETYPE_XLS : (spath == "xlsx" ? QETFILETYPE_XLSX : -1)));
			if (ftype == -1)
			{
				QMessageBox::critical(this, "ERROR", "Unknown Error");
				QApplication::quit();
			}
			err = QETERROR_NOERROR;
			xfile = fxls;
			CreateThread(NULL, 0, ReadFile, NULL, NULL, NULL);
		}
	}
}

inline void printqsl(QStringList src)
{
	for (int i = 0; i < src.size(); i++)
	{
		qDebug(src.at(i).toStdString().c_str())
	}
}

//new thread
DWORD WINAPI ReadFile(LPVOID param)
{
	xstatusBar->showMessage(QString::fromLocal8Bit("信息：读取文件..."));
	QString qs = xfile->readAll();
	xfile->close();
	QStringList qsl = qs.split('\n');
	size_t ssize = qs.length();
	int pos = 0, rmax = 0;
	for (int i = 0; i < qsl.size(); i++)
	{
		rmax = 0;
		for (int j = 0; j < qsl.at(i).length(); j++)
		{
			rmax += (qsl.at(i).at(j) == ',');
		}
		ntvrow = ntvrow > rmax ? ntvrow : rmax;
	}
	ntvline = qsl.size() - 2;
	//generate table head
	for (int i = 0; i < qsl.size(); i++)
	{
		QStringList lline = qsl.at(i).split(',');
		if (lline.size() > 2 && (((!lline.at(0).isEmpty()) || (!lline.at(0).isNull()))) && (lline.at(1).isEmpty() || lline.at(1).isNull()))
			qsl.removeAt(i);//remove useless line
	}
	for (int i = 0; i < qsl.size(); i++)
	{
		QStringList lline = qsl.at(i).split(',');
		for (int j = 0; j < lline.length(); j++)
		{
			if ((!lline.at(j).isEmpty()) && ((lline.at(j + 1).isEmpty())))
			{
				lline.insert(j + 1, lline.at(j));
				for (int k = 0; k < lline.size(); k++)
					qDebug("lline[%d]:%s",k,lline.at(k).toStdString().c_str());
			}

		}
	}

	qDebug("%d,%d", ntvrow, ntvline);

	xstatusBar->showMessage(QString::fromLocal8Bit("就绪"));
	return 0;
}
