#pragma once
#include <qstring.h>
#include <qtreewidget.h>
#include "io/tinyxml2.h"

QString IsValid(bool condition, bool& res);
QString XmlAttribute(tinyxml2::XMLElement* el, QString name, QString defValue = "");


